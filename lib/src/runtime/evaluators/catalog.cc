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

        // Get the default body attributes
        vector<pair<ast::attribute_operator, shared_ptr<attribute>>> default_attributes;
        if (auto default_body = find_default_body(expr)) {
            default_attributes = evaluate_attributes(is_class, default_body->attributes());
        }

        // Create the resources in the expression
        vector<resource*> resources = create_resources(is_class, type_name, expr, default_attributes);

        // Declare classes now
        // Defined types will be declared upon catalog finalization to allow for support of resource attribute defaults and overrides
        if (is_class) {
            auto& context = _evaluator.evaluation_context();
            auto catalog = context.catalog();
            for (auto resource : resources) {
                catalog->declare_class(context, resource->type(), resource->context(), resource->position());
            }
        }

        // Return an array of the resource types
        values::array types;
        types.reserve(resources.size());
        for (auto resource : resources) {
            types.push_back(resource->type());
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
        auto& context = _evaluator.evaluation_context();
        auto catalog = context.catalog();
        auto reference = _evaluator.evaluate(expr.reference());
        auto position = get_position(expr.reference());

        // Convert the value into an array of resource pointers
        vector<resource*> resources;
        if (!for_each<values::type>(reference, [&](values::type& resource_reference) {
            // Check for Class types
            if (boost::get<types::klass>(&resource_reference)) {
                throw _evaluator.create_exception(position, "cannot override attributes of a class resource.");
            }

            // Make sure the type is a qualified Resource type
            auto resource_type = boost::get<types::resource>(&resource_reference);
            if (!resource_type) {
                throw _evaluator.create_exception(position, (boost::format("expected qualified %1% but found %2%.") % types::resource::name() % get_type(resource_reference)).str());
            }

            if (!resource_type->fully_qualified()) {
                // TODO: implement
                throw _evaluator.create_exception(expr.position(), "resource defaults expressions are not yet implemented.");
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

        auto attributes = evaluate_attributes(false, expr.attributes());

        // Loop through each attribute
        for (auto& kvp : attributes) {
            auto op = kvp.first;
            auto& attribute = kvp.second;

            // Set or append the attribute on each resource
            for (size_t i = 0; i < resources.size(); ++i) {
                auto& resource = *resources[i];

                // Walk the parent scope looking for an associated resource that contains this one
                bool override = false;
                auto parent = context.current_scope()->parent().get();
                while (parent) {
                    if (parent->resource() && catalog->is_contained(resource, *parent->resource())) {
                        override = true;
                        break;
                    }
                    parent = parent->parent().get();
                }

                if (op == ast::attribute_operator::assignment) {
                    if (!override && resource.get(attribute->name())) {
                        if (is_undef(*attribute->value())) {
                            throw _evaluator.create_exception(
                                attribute->name_position(),
                                (boost::format("cannot remove attribute '%1%' from resource %2%.") %
                                 attribute->name() %
                                 resource.type()
                                ).str());
                        }
                        throw _evaluator.create_exception(
                            attribute->name_position(),
                            (boost::format("cannot override attribute '%1%' because it has already been set for resource %2%.") %
                             attribute->name() %
                             resource.type()
                            ).str());
                    }
                    // Set the attribute on the resource
                    resource.set(attribute);
                } else if (op == ast::attribute_operator::append) {
                    if (!override && resource.get(attribute->name())) {
                        throw _evaluator.create_exception(
                            attribute->name_position(),
                            (boost::format("cannot append to attribute '%1%' because it has already been set for resource %2%.") %
                             attribute->name() %
                             resource.type()
                            ).str());
                    }
                    resource.append(attribute);
                } else {
                    throw runtime_error("unexpected attribute operator");
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

    vector<pair<ast::attribute_operator, shared_ptr<attribute>>> catalog_expression_evaluator::evaluate_attributes(bool is_class, optional<vector<ast::attribute_expression>> const& expressions)
    {
        vector<pair<ast::attribute_operator, shared_ptr<attribute>>> attributes;
        if (!expressions) {
            return attributes;
        }

        // If there's a default resource body, set the attributes
        unordered_set<string> names;
        for (auto const& expression : *expressions) {
            auto& name = expression.name().value();

            // Splat the attribute if named '*'
            if (name == "*") {
                splat_attribute(attributes, names, expression);
                continue;
            }

            // Check for the "stage" attribute for non-classes
            if (!is_class && name == "stage") {
                throw _evaluator.create_exception(expression.position(), (boost::format("attribute '%1%' is only valid for classes.") % name).str());
            }

            if (!names.insert(name).second) {
                throw _evaluator.create_exception(expression.position(), (boost::format("attribute '%1%' already exists in this resource body.") % name).str());
            }

            // Evaluate and validate the attribute value
            auto value = _evaluator.evaluate(expression.value());
            validate_attribute(expression.value().position(), name, value);

            // Add an attribute to the list
            attributes.emplace_back(make_pair(expression.op(), std::make_shared<runtime::attribute>(
                _evaluator.compilation_context(),
                name,
                expression.name().position(),
                std::make_shared<values::value>(rvalue_cast(value)),
                expression.value().position()
            )));
        }
        return attributes;
    }

    void catalog_expression_evaluator::validate_attribute(lexer::position const& position, string const& name, values::value& value)
    {
        // Type information for metaparameters
        static const values::type string_array_type = types::array(types::string());
        static const values::type relationship_type = types::array(types::variant({ types::string(), types::catalog_entry() }));
        static const values::type string_type = types::string();
        static const values::type boolean_type = types::boolean();
        static const values::type loglevel_type = types::enumeration(vector<string>({ "debug", "info", "notice", "warning", "err", "alert", "emerg", "crit", "verbose" }));
        static const values::type audit_type = types::variant({ types::string(), string_array_type });

        // Evaluate the value expression
        bool converted = false;

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
        }

        // Validate the type of the parameter
        if (type && !is_instance(value, *type)) {
            throw _evaluator.create_exception(position, (boost::format("expected %1% for attribute '%2%' but found %3%.") % *type % name % get_type(converted ? as<values::array>(value)->at(0) : value)).str());
        }
    }

    void catalog_expression_evaluator::splat_attribute(
        vector<pair<ast::attribute_operator, shared_ptr<attribute>>>& attributes,
        unordered_set<string>& names,
        ast::attribute_expression const& attribute)
    {
        // Evaluate what must be a hash
        auto value = _evaluator.evaluate(attribute.value());
        if (!as<values::hash>(value)) {
            throw _evaluator.create_exception(attribute.value().position(), (boost::format("expected a %1% but found %2%.") % types::hash::name() % get_type(value)).str());
        }

        // Set each element of the hash as an attribute
        auto hash = mutate_as<values::hash>(value);
        for (auto& element : hash) {
            auto name = as<string>(element.first);
            if (!name) {
                throw _evaluator.create_exception(attribute.value().position(), (boost::format("expected all keys in hash to be %1% but found %2%.") % types::string::name() % get_type(element.first)).str());
            }
            if (!names.insert(*name).second) {
                throw _evaluator.create_exception(attribute.position(), (boost::format("attribute '%1%' already exists in this resource body.") % *name).str());
            }

            // Validate the attribute value
            validate_attribute(attribute.value().position(), *name, element.second);

            // Add the attribute to the list
            attributes.emplace_back(make_pair(attribute.op(), std::make_shared<runtime::attribute>(
                _evaluator.compilation_context(),
                *name,
                attribute.name().position(),
                std::make_shared<values::value>(rvalue_cast(element.second)),
                attribute.value().position()
            )));
        }
    }

    vector<resource*> catalog_expression_evaluator::create_resources(
        bool is_class,
        string const& type_name,
        ast::resource_expression const& expression,
        vector<pair<ast::attribute_operator, shared_ptr<attribute>>> const& default_attributes)
    {
        auto& evaluation_context = _evaluator.evaluation_context();
        auto& compilation_context = _evaluator.compilation_context();
        auto catalog = evaluation_context.catalog();

        // If a class, don't set a container; one will be set when the class is declared
        resource const* container = is_class ? nullptr : evaluation_context.current_scope()->resource();

        vector<resource*> resources;
        for (auto const& body : expression.bodies()) {
            auto title = _evaluator.evaluate(body.title());

            // If the default title, ignore (we've already evaluated the default attributes)
            if (is_default(title)) {
                continue;
            }

            // Evaluate the attributes
            auto attributes = evaluate_attributes(is_class, body.attributes());

            // Add each resource to the catalog
            if (!for_each<string>(title, [&](string& resource_title) {
                if (resource_title.empty()) {
                    throw _evaluator.create_exception(body.position(), "resource title cannot be empty.");
                }

                // Add the resource to the catalog
                auto& resource = catalog->add_resource(
                    types::resource(type_name, rvalue_cast(resource_title)),
                    compilation_context,
                    body.position(),
                    container);

                // Set the default attributes
                set_attributes(resource, default_attributes);

                // Set the resource's attributes
                set_attributes(resource, attributes);

                // Add the resource to the list
                resources.emplace_back(&resource);
            })) {
                throw _evaluator.create_exception(body.position(), (boost::format("expected %1% or %2% for resource title.") % types::string::name() % types::array(types::string())).str());
            }
        }
        return resources;
    }

    void catalog_expression_evaluator::set_attributes(runtime::resource& resource, vector<pair<ast::attribute_operator, shared_ptr<attribute>>> const& attributes)
    {
        // Set the default attributes
        for (auto& kvp : attributes) {
            auto op = kvp.first;
            auto& attribute = kvp.second;

            // Only support assignment
            if (op != ast::attribute_operator::assignment) {
                throw _evaluator.create_exception(
                    attribute->name_position(),
                    (boost::format("illegal attribute operation '%1%': only '%2%' is supported in a resource expression.") %
                     op %
                     ast::attribute_operator::assignment
                    ).str());
            }
            resource.set(attribute);
        }
    }

}}}  // namespace puppet::runtime::evaluators
