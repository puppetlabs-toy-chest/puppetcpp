#include <puppet/runtime/evaluators/primary.hpp>
#include <puppet/runtime/evaluators/basic.hpp>
#include <puppet/runtime/evaluators/catalog.hpp>
#include <puppet/runtime/evaluators/control_flow.hpp>
#include <puppet/runtime/evaluators/postfix.hpp>
#include <puppet/runtime/operators/logical_not.hpp>
#include <puppet/runtime/operators/negate.hpp>
#include <puppet/runtime/operators/splat.hpp>
#include <puppet/ast/expression_def.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace evaluators {

    primary_expression_evaluator::primary_expression_evaluator(expression_evaluator& evaluator, ast::primary_expression const& expression) :
        _evaluator(evaluator),
        _expression(expression)
    {
    }

    primary_expression_evaluator::result_type primary_expression_evaluator::evaluate()
    {
        return boost::apply_visitor(*this, _expression);
    }

    primary_expression_evaluator::result_type primary_expression_evaluator::operator()(boost::blank const&)
    {
        return value();
    }

    primary_expression_evaluator::result_type primary_expression_evaluator::operator()(ast::basic_expression const& expr)
    {
        evaluators::basic_expression_evaluator evaluator(_evaluator, expr);
        return evaluator.evaluate();
    }

    primary_expression_evaluator::result_type primary_expression_evaluator::operator()(ast::control_flow_expression const& expr)
    {
        evaluators::control_flow_expression_evaluator evaluator(_evaluator, expr);
        return evaluator.evaluate();
    }

    primary_expression_evaluator::result_type primary_expression_evaluator::operator()(ast::catalog_expression const& expr)
    {
        evaluators::catalog_expression_evaluator evaluator(_evaluator, expr);
        return evaluator.evaluate();
    }

    primary_expression_evaluator::result_type primary_expression_evaluator::operator()(ast::unary_expression const& expr)
    {
        static const unordered_map<ast::unary_operator, std::function<values::value(operators::unary_context&)>> unary_operators = {
            { ast::unary_operator::negate,      operators::negate() },
            { ast::unary_operator::logical_not, operators::logical_not() },
            { ast::unary_operator::splat,       operators::splat() }
        };

        auto it = unary_operators.find(expr.op());
        if (it == unary_operators.end()) {
            throw _evaluator.create_exception(expr.position(), "unexpected unary expression.");
        }

        auto operand = boost::apply_visitor(*this, expr.operand());
        operators::unary_context context(_evaluator, operand, expr.position());
        return it->second(context);
    }

    primary_expression_evaluator::result_type primary_expression_evaluator::operator()(ast::postfix_expression const& expr)
    {
        evaluators::postfix_expression_evaluator evaluator(_evaluator, expr);
        return evaluator.evaluate();
    }

    primary_expression_evaluator::result_type primary_expression_evaluator::operator()(ast::expression const& expr)
    {
        return _evaluator.evaluate(expr);
    }

}}}  // namespace puppet::runtime::evaluators
