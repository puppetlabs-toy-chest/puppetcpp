#include <puppet/runtime/evaluators/catalog.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace evaluators {

    catalog_expression_evaluator::catalog_expression_evaluator(expression_evaluator& evaluator, ast::catalog_expression const& expression) :
        _evaluator(evaluator),
        _expression(expression)
    {
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::evaluate()
    {
        return boost::apply_visitor(*this, _expression);
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::resource_expression const& expr)
    {
        auto& catalog = _evaluator.catalog();
        string const& type_name = expr.type().value();

        // Handle class resources specially
        if (type_name == "class") {
            return declare_classes(expr);
        }

        if (expr.status() == ast::resource_status::virtualized) {
            // TODO: add to a list of virtual resources
            throw evaluation_exception(expr.position(), "virtual resource expressions are not yet implemented.");
        }
        if (expr.status() == ast::resource_status::exported) {
            // TODO: add to a list of virtual exported resources
            throw evaluation_exception(expr.position(), "exported resource expressions are not yet implemented.");
        }

        // Handle defined types
        if (catalog.is_defined_type(type_name)) {
            return declare_defined_types(expr);
        }

        values::array types;
        vector<resource*> resources;
        for (auto const& body : expr.bodies()) {
            // Evaluate the title
            auto title = _evaluator.evaluate(body.title());

            // Add a resource for each string in the title
            resources.clear();
            if (!for_each<string>(title, [&](string& resource_title) {
                types::resource type(type_name, resource_title);
                if (type.title().empty()) {
                    throw evaluation_exception(body.position(), "resource title cannot be empty.");
                }

                // Add the resource to the catalog
                auto resource = catalog.add_resource(type, _evaluator.path(), body.position().line());
                if (!resource) {
                    resource = catalog.find_resource(type);
                    if (resource) {
                        throw evaluation_exception(body.position(), (boost::format("resource %1% was previously declared at %2%:%3%.") % type % resource->path() % resource->line()).str());
                    }
                    throw evaluation_exception(body.position(), (boost::format("failed to add resource %1% to catalog.") % type).str());
                }

                // Add the resource and type to the bookkeeping lists
                resources.push_back(resource);
                types.emplace_back(rvalue_cast(type));
            })) {
                throw evaluation_exception(body.position(), (boost::format("expected %1% or %2% for resource title.") % types::string::name() % types::array(types::string())).str());
            }
            if (!body.attributes()) {
                continue;
            }

            // Set the parameters
            for (auto const& attribute : *body.attributes()) {
                // Ensure only assignment for resource bodies
                if (attribute.op() != ast::attribute_operator::assignment) {
                    throw evaluation_exception(attribute.position(), (boost::format("illegal attribute opereration '%1%': only '%2%' is supported in a resource expression.") % attribute.op() % ast::attribute_operator::assignment).str());
                }

                // Evaluate the attribute value
                auto attribute_value = _evaluator.evaluate(attribute.value());

                // Loop through each resource in this body
                for (size_t i = 0; i < resources.size(); ++i) {
                    auto& resource = *resources[i];

                    // For the last resource, move the value; otherwise copy
                    values::value value;
                    if (i == resources.size() - 1) {
                        value = rvalue_cast(attribute_value);
                    } else {
                        value = attribute_value;
                    }

                    // Set the parameter in the resource
                    resource.set_parameter(attribute.name().value(), attribute.name().position(), rvalue_cast(value), attribute.value().position());
                }
            }
        }
        return types;
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::resource_defaults_expression const& expr)
    {
        // TODO: implement
        throw evaluation_exception(expr.position(), "resource defaults expressions are not yet implemented.");
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::resource_override_expression const& expr)
    {
        auto& catalog = _evaluator.catalog();
        auto reference = _evaluator.evaluate(expr.reference());
        auto position = get_position(expr.reference());

        // Convert the value into an array of resource pointers
        vector<resource*> resources;
        if (!for_each<values::type>(reference, [&](values::type& resource_reference) {
            // Make sure the type is a qualified Resource type
            auto resource_type = boost::get<types::resource>(&resource_reference);
            if (!resource_type || resource_type->type_name().empty() || resource_type->title().empty()) {
                throw evaluation_exception(position, (boost::format("expected qualified %1% but found %2%.") % types::resource::name() % get_type(resource_reference)).str());
            }

            // Find the resource
            auto resource = catalog.find_resource(*resource_type);
            if (!resource) {
                throw evaluation_exception(position, (boost::format("resource %1% does not exist in the catalog.") % *resource_type).str());
            }
            resources.push_back(resource);
        })) {
            throw evaluation_exception(position, (boost::format("expected %1% or %2% for resource reference.") % types::resource::name() % types::array(types::resource())).str());
        }

        if (expr.attributes()) {
            // Set the parameters
            for (auto const& attribute : *expr.attributes()) {
                // Evaluate the attribute value
                auto attribute_value = _evaluator.evaluate(attribute.value());

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

                    if (attribute.op() == ast::attribute_operator::assignment) {
                        if (is_undef(value)) {
                            if (!override) {
                                throw evaluation_exception(attribute.name().position(), (boost::format("cannot remove attribute '%1%' from resource %2%.") % attribute.name() % resource.type()).str());
                            }
                            resource.remove_parameter(attribute.name().value());
                            continue;
                        }
                        // Set the parameter in the resource
                        resource.set_parameter(attribute.name().value(), attribute.name().position(), rvalue_cast(value), attribute.value().position());
                    } else if (attribute.op() == ast::attribute_operator::append) {
                        // TODO: append parameter
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
        return types::klass(_evaluator.scope().qualify(expr.name().value()));
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::defined_type_expression const& expr)
    {
        // Defined type expressions are handled by the definition scanner
        // Just return a reference to the type
        return types::resource(expr.name().value());
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
        throw evaluation_exception(expr.position(), "collection expressions are not yet implemented.");
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::declare_classes(ast::resource_expression const& expr)
    {
        auto& catalog = _evaluator.catalog();

        if (expr.status() == ast::resource_status::virtualized) {
            throw evaluation_exception(expr.position(), "class resources cannot be virtual.");
        }
        if (expr.status() == ast::resource_status::exported) {
            throw evaluation_exception(expr.position(), "class resources cannot be exported.");
        }

        values::array types;
        vector<string> class_names;
        unordered_map<ast::name, values::value> attributes;
        values::array parameters;
        for (auto const& body : expr.bodies()) {
            // Evaluate the title as class names
            auto title = _evaluator.evaluate(body.title());
            class_names.clear();
            if (!for_each<string>(title, [&](string& resource_title) {
                class_names.emplace_back(rvalue_cast(resource_title));
            })) {
                throw evaluation_exception(body.position(), (boost::format("expected %1% or %2% for resource title.") % types::string::name() % types::array(types::string())).str());
            }

            // For this body, evaluate all attributes
            attributes.clear();
            if (body.attributes()) {
                for (auto const& attribute : *body.attributes()) {
                    // Ensure only assignment for resource bodies
                    if (attribute.op() != ast::attribute_operator::assignment) {
                        throw evaluation_exception(attribute.position(), (boost::format("illegal attribute opereration '%1%': only '%2%' is supported in a resource expression.") % attribute.op() % ast::attribute_operator::assignment).str());
                    }

                    // Evaluate the attribute value
                    if (!attributes.emplace(make_pair(attribute.name(), _evaluator.evaluate(attribute.value()))).second) {
                        throw evaluation_exception(attribute.position(), (boost::format("attribute '%1%' already exists in this resource body.") % attribute.name()).str());
                    }
                }
            }

            // Declare the classes
            for (auto& name : class_names) {
                types::klass klass(rvalue_cast(name));
                types::resource resource("class", klass.title());

                // Ensure the resource doesn't already exist
                if (auto existing = catalog.find_resource(resource)) {
                    throw evaluation_exception(body.position(), (boost::format("class '%1%' was already declared at %2%:%3%.") % klass.title() % existing->path() % existing->line()).str());
                }

                // Ensure the class is defined
                if (!catalog.is_class_defined(klass)) {
                    throw evaluation_exception(body.position(), (boost::format("cannot declare class '%1%' because the class has not been defined.") % klass.title()).str());
                }

                // Declare the class
                if (!catalog.declare_class(_evaluator.context(), klass, _evaluator.path(), body.position(), &attributes)) {
                    throw evaluation_exception(body.position(), (boost::format("failed to declare class '%1%'.") % klass.title()).str());
                }
                types.emplace_back(rvalue_cast(resource));
            }
        }
        return types;
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::declare_defined_types(ast::resource_expression const& expr)
    {
        auto& catalog = _evaluator.catalog();

        values::array types;
        vector<string> titles;
        unordered_map<ast::name, values::value> attributes;
        values::array parameters;
        for (auto const& body : expr.bodies()) {
            // Evaluate the titles
            auto title = _evaluator.evaluate(body.title());
            titles.clear();
            if (!for_each<string>(title, [&](string& resource_title) {
                titles.emplace_back(rvalue_cast(resource_title));
            })) {
                throw evaluation_exception(body.position(), (boost::format("expected %1% or %2% for resource title.") % types::string::name() % types::array(types::string())).str());
            }

            // For this body, evaluate all attributes
            attributes.clear();
            if (body.attributes()) {
                for (auto const& attribute : *body.attributes()) {
                    // Ensure only assignment for resource bodies
                    if (attribute.op() != ast::attribute_operator::assignment) {
                        throw evaluation_exception(attribute.position(), (boost::format("illegal attribute opereration '%1%': only '%2%' is supported in a resource expression.") % attribute.op() % ast::attribute_operator::assignment).str());
                    }

                    // Evaluate the attribute value
                    if (!attributes.emplace(make_pair(attribute.name(), _evaluator.evaluate(attribute.value()))).second) {
                        throw evaluation_exception(attribute.position(), (boost::format("attribute '%1%' already exists in this resource body.") % attribute.name()).str());
                    }
                }
            }

            // Declare the resources
            for (auto& title : titles) {
                types::resource resource(expr.type().value(), title);

                // Ensure the resource doesn't already exist
                if (auto existing = catalog.find_resource(resource)) {
                    throw evaluation_exception(body.position(), (boost::format("resource %1% was previously declared at %2%:%3%.") % resource % existing->path() % existing->line()).str());
                }

                // Declare the defined type
                if (!catalog.declare_defined_type(_evaluator.context(), expr.type().value(), title, _evaluator.path(), body.position(), &attributes)) {
                    throw evaluation_exception(body.position(), (boost::format("failed to declare defined type %1%.") % resource).str());
                }
                types.emplace_back(rvalue_cast(resource));
            }
        }
        return types;
    }

}}}  // namespace puppet::runtime::evaluators
