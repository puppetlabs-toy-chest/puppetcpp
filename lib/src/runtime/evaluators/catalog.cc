#include <puppet/runtime/evaluators/catalog.hpp>
#include <puppet/ast/expression_def.hpp>

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
        if (expr.status() == ast::resource_status::virtualized) {
            // TODO: add to a list of virtual resources
            throw evaluation_exception(expr.position(), "virtual resource expressions are not yet implemented.");
        }
        if (expr.status() == ast::resource_status::exported) {
            // TODO: add to a list of virtual exported resources
            throw evaluation_exception(expr.position(), "exported resource expressions are not yet implemented.");
        }

        values::array types;
        vector<resource*> resources;
        for (auto const& body : expr.bodies()) {
            // Add the resource(s) to the catalog
            resources.clear();
            add_resource(resources, types, expr.type().value(), _evaluator.evaluate(body.title()), body.position());
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
                        value = std::move(attribute_value);
                    } else {
                        value = attribute_value;
                    }

                    // Set the parameter in the resource
                    resource.set_parameter(attribute.name().value(), attribute.name().position(), std::move(value), attribute.value().position());
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
        // TODO: implement
        throw evaluation_exception(expr.position(), "resource override expressions are not yet implemented.");
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::class_definition_expression const& expr)
    {
        // TODO: implement
        throw evaluation_exception(expr.position(), "class expressions are not yet implemented.");
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::defined_type_expression const& expr)
    {
        // TODO: implement
        throw evaluation_exception(expr.position(), "defined type expressions are not yet implemented.");
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::node_definition_expression const& expr)
    {
        // TODO: implement
        throw evaluation_exception(expr.position(), "node definition expressions are not yet implemented.");
    }

    catalog_expression_evaluator::result_type catalog_expression_evaluator::operator()(ast::collection_expression const& expr)
    {
        // TODO: implement
        throw evaluation_exception(expr.position(), "collection expressions are not yet implemented.");
    }

    void catalog_expression_evaluator::add_resource(vector<resource*>& resources, values::array& types, string const& type_name, values::value title, token_position const& position)
    {
        if (as<string>(title)) {
            // Create a type for this resource
            types::resource type(type_name, mutate_as<string>(title));
            if (type.title().empty()) {
                throw evaluation_exception(position, "resource title cannot be empty.");
            }

            // Add the resource to the catalog
            auto& catalog = _evaluator.catalog();
            auto resource = catalog.add_resource(type.type_name(), type.title(), _evaluator.path(), std::get<1>(position));
            if (!resource) {
                resource = catalog.find_resource(type.type_name(), type.title());
                if (resource) {
                    throw evaluation_exception(position, (boost::format("resource %1% was previously declared at %2%:%3%.") % type % resource->file() % resource->line()).str());
                }
                throw evaluation_exception(position, (boost::format("resource %1% already exists in the catalog.") % type).str());
            }

            // Add the resource and type to the bookkeeping lists
            resources.push_back(resource);
            types.emplace_back(std::move(type));
            return;
        }
        if (as<values::array>(title)) {
            // For arrays, recurse on each element
            auto titles = mutate_as<values::array>(title);
            for (auto& element : titles) {
                add_resource(resources, types, type_name, std::move(element), position);
            }
            return;
        }
        throw evaluation_exception(position, (boost::format("expected %1% resource title but found %2%.") % types::string::name() % get_type(title)).str());
    }

}}}  // namespace puppet::runtime::evaluators
