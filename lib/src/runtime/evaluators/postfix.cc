#include <puppet/runtime/evaluators/postfix.hpp>
#include <puppet/runtime/evaluators/access.hpp>
#include <puppet/runtime/dispatcher.hpp>
#include <puppet/ast/expression_def.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace evaluators {

    postfix_expression_evaluator::postfix_expression_evaluator(expression_evaluator& evaluator, ast::postfix_expression const& expression) :
        _evaluator(evaluator),
        _expression(expression),
        _first_expression(nullptr)
    {
    }

    postfix_expression_evaluator::result_type postfix_expression_evaluator::evaluate()
    {
        // Evaluate the primary expression
        _result = _evaluator.evaluate(_expression.primary());
        _position = get_position(_expression.primary());
        _first_expression = &_expression.primary();

        // Loop through each access subexpression, replacing the result each time
        for (auto const& subexpression : _expression.subexpressions()) {
            _result = boost::apply_visitor(*this, subexpression);
            _first_expression = nullptr;
        }
        return _result;
    }

    postfix_expression_evaluator::result_type postfix_expression_evaluator::operator()(ast::selector_expression const& expr)
    {
        // Selector expressions create a new match scope
        match_variable_scope match_scope(_evaluator.scope());

        boost::optional<size_t> default_index;

        auto& cases = expr.cases();
        for (size_t i = 0; i < cases.size(); ++i) {
            auto& selector_case = cases[i];

            // Evaluate the option
            value selector = _evaluator.evaluate(selector_case.selector());
            if (is_default(selector)) {
                // Remember where the default case is and keep going
                default_index = i;
                continue;
            }

            // If unfolding, treat each element as an option
            auto unfold_array = _evaluator.unfold(selector_case.selector(), selector);
            if (unfold_array) {
                for (auto& element : *unfold_array) {
                    if (_evaluator.is_match(_result, expr.position(), element, selector_case.position())) {
                        _position = selector_case.position();
                        return _evaluator.evaluate(selector_case.result());
                    }
                }
            }

            if (_evaluator.is_match(_result, expr.position(), selector, selector_case.position())) {
                _position = selector_case.position();
                return _evaluator.evaluate(selector_case.result());
            }
        }

        // Handle no matching case
        if (!default_index) {
            throw evaluation_exception(expr.position(), (boost::format("no matching selector case for value '%1%'.") % _result).str());
        }

        // Evaluate the default case
        _position = cases[*default_index].position();
        return _evaluator.evaluate(cases[*default_index].result());
    }

    postfix_expression_evaluator::result_type postfix_expression_evaluator::operator()(ast::access_expression const& expr)
    {
        evaluators::access_expression_evaluator evaluator(_evaluator, expr);
        _position = expr.position();
        return evaluator.evaluate(dereference(_result));
    }

    postfix_expression_evaluator::result_type postfix_expression_evaluator::operator()(ast::method_call_expression const& expr)
    {
        runtime::dispatcher dispatcher(expr.method().value(), expr.method().position());

        // Evaluate the result for the next call
        value result = dispatcher.dispatch(_evaluator, expr.arguments(), expr.lambda(), &_result, _first_expression, &_position);
        _position = expr.position();
        return result;
    }

}}}  // namespace puppet::runtime::evaluators
