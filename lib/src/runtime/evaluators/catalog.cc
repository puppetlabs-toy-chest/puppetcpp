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

        // Realize the resources
        values::array result;
        auto& catalog = _evaluator.catalog();
        for (auto const& body : expr.bodies()) {
            // Evaluate the resource title
            auto title = _evaluator.evaluate(body.title());
            if (!as<string>(title)) {
                throw evaluation_exception(body.position(), (boost::format("expected %1% for resource title but found %2%.") % types::string::name() % get_type(title)).str());
            }

            types::resource resource_type(expr.type().value(), mutate_as<string>(title));
            if (resource_type.title().empty()) {
                throw evaluation_exception(body.position(), "resource title cannot be empty.");
            }

            // Check to see if the resource already exists
            resource* resource = nullptr;
            if ((resource = catalog.find_resource(resource_type.type_name(), resource_type.title()))) {
                throw evaluation_exception(body.position(), (boost::format("resource %1% was previously declared at %2%:%3%.") % resource_type % resource->file() % resource->line()).str());
            }

            // Add the resource to the catalog
            resource = catalog.add_resource(resource_type.type_name(), resource_type.title(), _evaluator.path(), std::get<1>(body.position()));

            // Set the parameters
            if (body.attributes()) {
                for (auto const& attribute : *body.attributes()) {
                    // TODO: handle meta-parameter
                    if (attribute.op() != ast::attribute_operator::assignment) {
                        throw evaluation_exception(attribute.position(), (boost::format("illegal attribute opereration '%1%': only '%2%' is supported in a resource expression.") % attribute.op() % ast::attribute_operator::assignment).str());
                    }
                    if (!resource->set_parameter(attribute.name().value(), _evaluator.evaluate(attribute.value()))) {
                        throw evaluation_exception(attribute.position(), (boost::format("attribute '%1%' has already been set in this resource body.") % attribute.name()).str());
                    }
                }
            }

            // Add the resource type to the result
            result.emplace_back(std::move(resource_type));
        }
        return result;
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
}}}  // namespace puppet::runtime::evaluators
