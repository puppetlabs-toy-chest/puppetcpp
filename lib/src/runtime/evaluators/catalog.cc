#include <puppet/runtime/evaluators/catalog.hpp>
#include <puppet/runtime/definition_scanner.hpp>
#include <puppet/runtime/executor.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;
using boost::optional;

namespace puppet { namespace runtime { namespace evaluators {

    catalog_expression_evaluator::catalog_expression_evaluator(expression_evaluator& evaluator, ast::catalog_expression const& expression) :
        _evaluator(evaluator),
        _expression(expression)
    {
        if (!_evaluator.evaluation_context().catalog()) {
            throw _evaluator.create_exception(get_position(_expression), "catalog expressions are not supported.");
        }
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::evaluate()
    {
        return boost::apply_visitor(*this, _expression);
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::resource_expression const& expr)
    {
        // Evaluate the type name
        string type_name;
        auto type_value = _evaluator.evaluate(expr.type());

        // Resource expressions support either strings or Resource[Type] for the type name
        bool is_class = false;
        if (as<std::string>(type_value)) {
            type_name = mutate_as<string>(type_value);
            is_class = type_name == "class";
        } else if (auto type = as<values::type>(type_value)) {
            if (auto resource_type = boost::get<types::resource>(type)) {
                if (resource_type->title().empty()) {
                    type_name = resource_type->type_name();
                    is_class = resource_type->is_class();
                }
            }
        }

        // Ensure there was a valid type name
        if (type_name.empty()) {
            throw _evaluator.create_exception(expr.position(), (boost::format("expected %1% or qualified %2% for resource type but found %3%.") % types::string::name() % types::resource::name() % get_type(type_value)).str());
        }

        if (expr.status() == ast::resource_status::virtualized) {
            // TODO: add to a list of virtual resources
            throw _evaluator.create_exception(expr.position(), "virtual resource expressions are not yet implemented.");
        }
        if (expr.status() == ast::resource_status::exported) {
            // TODO: add to a list of virtual exported resources
            throw _evaluator.create_exception(expr.position(), "exported resource expressions are not yet implemented.");
        }

        // TODO: check for known type
        // TODO: if not a known type and not a "class" resource, load the defined type
        // TODO: if type still unknown, raise an error
        auto& evaluation_context = _evaluator.evaluation_context();
        auto& compilation_context = _evaluator.compilation_context();
        auto catalog = evaluation_context.catalog();
        bool is_defined_type = !is_class && catalog->find_defined_type(type_name);

        // Evaluate the default attributes
        auto default_body = find_default_body(expr);
        auto defaults = evaluate_attributes(default_body);

        // Evaluate each body
        values::array types;
        for (auto const& body : expr.bodies()) {
            // Evaluate the title
            auto title = _evaluator.evaluate(body.title());

            // Check for the default resource body and skip
            if (is_default(title)) {
                continue;
            }

            // Evaluate the attributes
            auto attributes = evaluate_attributes(&body, defaults);

            try {
                // Add each resource to the catalog
                if (!for_each<string>(title, [&](string& resource_title) {
                    if (resource_title.empty()) {
                        throw _evaluator.create_exception(body.position(), "resource title cannot be empty.");
                    }

                    types::resource type(type_name, rvalue_cast(resource_title));
                    if (is_class) {
                        // Declare the class
                        catalog->declare_class(evaluation_context, type, compilation_context, body.position(), attributes);
                    } else if (is_defined_type) {
                        // Declare the defined type
                        catalog->declare_defined_type(evaluation_context, type_name, type, compilation_context, body.position(), attributes);
                    } else {
                        // Add the resource to the catalog
                        catalog->add_resource(evaluation_context, type, compilation_context, body.position(), attributes);
                    }

                    // Add the type to the return value
                    types.emplace_back(rvalue_cast(type));
                })) {
                    throw _evaluator.create_exception(body.position(), (boost::format("expected %1% or %2% for resource title.") % types::string::name() % types::array(types::string())).str());
                }
            } catch (attribute_name_exception const& ex) {
                throw _evaluator.create_exception(find_attribute_position(false, ex.name(), body, default_body), ex.what());
            } catch (attribute_value_exception const& ex) {
                throw _evaluator.create_exception(find_attribute_position(true, ex.name(), body, default_body), ex.what());
            }
        }
        return types;
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::resource_defaults_expression const& expr)
    {
        // TODO: implement
        throw _evaluator.create_exception(expr.position(), "resource defaults expressions are not yet implemented.");
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::resource_override_expression const& expr)
    {
        auto catalog = _evaluator.evaluation_context().catalog();
        auto reference = _evaluator.evaluate(expr.reference());
        auto position = get_position(expr.reference());

        // Convert the value into an array of resource pointers
        vector<resource*> resources;
        if (!for_each<values::type>(reference, [&](values::type& resource_reference) {
            // Make sure the type is a qualified Resource type
            auto resource_type = boost::get<types::resource>(&resource_reference);
            if (!resource_type || !resource_type->fully_qualified()) {
                throw _evaluator.create_exception(position, (boost::format("expected qualified %1% but found %2%.") % types::resource::name() % get_type(resource_reference)).str());
            }

            // Classes cannot be overridden
            if (resource_type->is_class()) {
                throw _evaluator.create_exception(position, "cannot override attributes of a class resource.");
            }

            // Find the resource
            auto resource = catalog->find_resource(*resource_type);
            if (!resource) {
                throw _evaluator.create_exception(position, (boost::format("resource %1% does not exist in the catalog.") % *resource_type).str());
            }
            resources.push_back(resource);
        })) {
            throw _evaluator.create_exception(position, (boost::format("expected %1% or %2% for resource reference.") % types::resource::name() % types::array(types::resource())).str());
        }

        if (expr.attributes()) {
            // Set the parameters
            for (auto const& attribute : *expr.attributes()) {

                auto const& name = attribute.name();

                // Evaluate the attribute value
                auto attribute_value = evaluate_attribute(attribute);

                // Loop through each resource
                for (size_t i = 0; i < resources.size(); ++i) {
                    auto& resource = *resources[i];

                    // TODO: check the resource scope; if the current scope inherits from the resource's scope, allow overriding or removing of parameters
                    bool override = false;

                    // For the last resource, move the value; otherwise copy
                    values::value value;
                    if (i == resources.size() - 1) {
                        value = rvalue_cast(attribute_value);
                    } else {
                        value = attribute_value;
                    }

                    // Make the resource attribute's unique as they're about to change
                    resource.make_attributes_unique();
                    auto& attributes = resource.attributes();

                    if (attribute.op() == ast::attribute_operator::assignment) {
                        if (!override && attributes.get(name.value())) {
                            if (is_undef(value)) {
                                throw _evaluator.create_exception(name.position(), (boost::format("cannot remove attribute '%1%' from resource %2%.") % name % resource.type()).str());
                            }
                            throw _evaluator.create_exception(name.position(), (boost::format("attribute '%1%' has already been set for resource %2%.") % name % resource.type()).str());
                        }
                        // Set the parameter in the resource
                        attributes.set(name.value(), rvalue_cast(value));
                    } else if (attribute.op() == ast::attribute_operator::append) {
                        if (!override && attributes.get(name.value())) {
                            throw _evaluator.create_exception(name.position(), (boost::format("attribute '%1%' has already been set for resource %2% and cannot be appended to.") % name % resource.type()).str());
                        }
                        if (!attributes.append(name.value(), rvalue_cast(value))) {
                            throw _evaluator.create_exception(name.position(), (boost::format("attribute '%1%' is not an array.") % name).str());
                        }
                    } else {
                        throw runtime_error("invalid attribute operator");
                    }
                }
            }
        }

        return reference;
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::class_definition_expression const& expr)
    {
        // Class definitions are handled by the definition scanner
        // Just return a reference to the class
        return types::klass(_evaluator.evaluation_context().current_scope()->qualify(expr.name().value()));
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::defined_type_expression const& expr)
    {
        // Defined type expressions are handled by the definition scanner
        // Just return a reference to the type
        return types::resource(_evaluator.evaluation_context().current_scope()->qualify(expr.name().value()));
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::node_definition_expression const& expr)
    {
        // Node definition expressions are handled by the definition scanner
        // Just return undef
        return values::undef();
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::collection_expression const& expr)
    {
        // TODO: implement
        throw _evaluator.create_exception(expr.position(), "collection expressions are not yet implemented.");
    }

    bool catalog_expression_evaluator::is_default_expression(ast::primary_expression const& expr)
    {
        if (auto ptr = boost::get<ast::expression>(&expr)) {
            return ptr->binary().empty() && is_default_expression(ptr->primary());
        }
        if (auto ptr = boost::get<ast::basic_expression>(&expr)) {
            return boost::get<ast::defaulted>(ptr);
        }
        return false;
    }

    ast::resource_body const* catalog_expression_evaluator::find_default_body(ast::resource_expression const& expr)
    {
        ast::resource_body const* default_body = nullptr;
        for (auto const& body : expr.bodies()) {
            if (!is_default_expression(body.title())) {
                continue;
            }
            if (default_body) {
                throw _evaluator.create_exception(body.position(), "only one default body is supported in a resource expression.");
            }
            default_body = &body;
        }
        return default_body;
    }

    shared_ptr<runtime::attributes> catalog_expression_evaluator::evaluate_attributes(ast::resource_body const* body, shared_ptr<runtime::attributes const> parent)
    {
        shared_ptr<runtime::attributes> attributes;
        if (body && body->attributes() && !body->attributes()->empty()) {
            attributes = make_shared<runtime::attributes>(rvalue_cast(parent));
            for (auto const& attribute : *body->attributes()) {
                // Ensure only assignment for resource bodies
                if (attribute.op() != ast::attribute_operator::assignment) {
                    throw _evaluator.create_exception(attribute.position(), (boost::format("illegal attribute operation '%1%': only '%2%' is supported in a resource expression.") % attribute.op() % ast::attribute_operator::assignment).str());
                }

                // Ensure the value doesn't exist locally on the attributes collection
                if (attributes->get(attribute.name().value(), false)) {
                    throw _evaluator.create_exception(attribute.position(), (boost::format("attribute '%1%' already exists in this resource body.") % attribute.name()).str());
                }

                // Set the attribute
                attributes->set(attribute.name().value(), evaluate_attribute(attribute));
            }
        } else if (parent) {
            attributes = make_shared<runtime::attributes>(rvalue_cast(parent));
        }
        return attributes;
    }

    values::value catalog_expression_evaluator::evaluate_attribute(ast::attribute_expression const& attribute)
    {
        // Type information for metaparameters
        static const values::type string_array_type = types::array(types::string());
        static const values::type relationship_type = types::array(types::variant({ types::string(), types::catalog_entry() }));
        static const values::type string_type = types::string();
        static const values::type boolean_type = types::boolean();
        static const values::type loglevel_type = types::enumeration(vector<string>({ "debug", "info", "notice", "warning", "err", "alert", "emerg", "crit", "verbose" }));
        static const values::type audit_type = types::variant({ types::string(), string_array_type });

        // Evaluate the value expression
        auto value = _evaluator.evaluate(attribute.value());
        bool converted = false;

        auto const& name = attribute.name().value();

        // Perform metaparameter checks
        values::type const* type = nullptr;
        if (name == "alias") {
            type = &string_array_type;
            converted = !as<values::array>(value);
            value = to_array(value, false);
        } else if (name == "audit") {
            type = &audit_type;
        } else if (name == "before" || name == "notify" || name == "require" || name == "subscribe") {
            type = &relationship_type;
            converted = !as<values::array>(value);
            value = to_array(value, false);
        } else if (name == "loglevel") {
            type = &loglevel_type;
        } else if (name == "noop") {
            type = &boolean_type;
        } else if (name == "schedule") {
            type = &string_type;
        } else if (name == "stage") {
            type = &string_type;
        } else if (name == "tag") {
            type = &string_array_type;
            converted = !as<values::array>(value);
            value = to_array(value, false);
        }

        if (!type) {
            // Not a metaparameter
            // TODO: get the attribute type from the type's definition and validate
            return value;
        }

        // Validate the type of the parameter
        if (!is_instance(value, *type)) {
            throw _evaluator.create_exception(attribute.value().position(), (boost::format("expected %1% for attribute '%2%' but found %3%.") % *type % name % get_type(converted ? as<values::array>(value)->at(0) : value)).str());
        }
        return value;
    }

    lexer::position catalog_expression_evaluator::find_attribute_position(bool for_value, string const& name, ast::resource_body const& current, ast::resource_body const* default_body)
    {
        if (current.attributes()) {
            for (auto const& attribute : *current.attributes()) {
                if (attribute.name().value() == name) {
                    return for_value ? attribute.value().position() : attribute.name().position();
                }
            }
        }
        if (default_body && default_body->attributes()) {
            for (auto const& attribute : *default_body->attributes()) {
                if (attribute.name().value() == name) {
                    return for_value ? attribute.value().position() : attribute.name().position();
                }
            }
        }
        throw runtime_error("unexpected attribute name for position.");
    }


}}}  // namespace puppet::runtime::evaluators
