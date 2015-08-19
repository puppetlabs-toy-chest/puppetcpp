#include <puppet/runtime/evaluators/control_flow.hpp>
#include <puppet/runtime/dispatcher.hpp>
#include <puppet/ast/expression_def.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace evaluators {

    control_flow_expression_evaluator::control_flow_expression_evaluator(expression_evaluator& evaluator, ast::control_flow_expression const& expression) :
        _evaluator(evaluator),
        _expression(expression)
    {
    }

    control_flow_expression_evaluator::result_type control_flow_expression_evaluator::evaluate()
    {
        return boost::apply_visitor(*this, _expression);
    }

    control_flow_expression_evaluator::result_type control_flow_expression_evaluator::operator()(ast::case_expression const& expr)
    {
        // Case expressions create a new match scope
        auto match_scope = _evaluator.evaluation_context().create_match_scope();

        // Evaluate the case's expression
        value result = _evaluator.evaluate(expr.expression());

        auto& propositions = expr.propositions();
        boost::optional<size_t> default_index;
        for (size_t i = 0; i < propositions.size(); ++i) {
            auto& proposition = propositions[i];

            // Look for a match in the options
            for (auto& option : proposition.options()) {
                // Evaluate the option
                value option_value = _evaluator.evaluate(option);
                if (is_default(option_value)) {
                    // Remember where the default is and keep going
                    default_index = i;
                    continue;
                }

                // If unfolding, treat each element as an option
                auto unfold_array = _evaluator.unfold(option, option_value);
                if (unfold_array) {
                    for (auto& element : *unfold_array) {
                        if (_evaluator.is_match(result, expr.position(), element, option.position())) {
                            return execute_block(proposition.body());
                        }
                    }
                }

                if (_evaluator.is_match(result, expr.position(), option_value, option.position())) {
                    return execute_block(proposition.body());
                }
            }
        }

        // Handle no matching case
        if (default_index) {
            return execute_block(propositions[*default_index].body());
        }

        // Nothing matched, return undef
        return value();
    }

    control_flow_expression_evaluator::result_type control_flow_expression_evaluator::operator()(ast::if_expression const& expr)
    {
        // If expressions create a new match scope
        auto match_scope = _evaluator.evaluation_context().create_match_scope();

        if (is_truthy(_evaluator.evaluate(expr.conditional()))) {
            return execute_block(expr.body());
        }
        if (expr.elsifs()) {
            for (auto& elsif : *expr.elsifs()) {
                if (is_truthy(_evaluator.evaluate(elsif.conditional()))) {
                    return execute_block(elsif.body());
                }
            }
        }
        if (expr.else_()) {
            return execute_block(expr.else_()->body());
        }
        return value();
    }

    control_flow_expression_evaluator::result_type control_flow_expression_evaluator::operator()(ast::unless_expression const& expr)
    {
        // Unless expressions create a new match scope
        auto match_scope = _evaluator.evaluation_context().create_match_scope();

        if (!is_truthy(_evaluator.evaluate(expr.conditional()))) {
            return execute_block(expr.body());
        }
        if (expr.else_()) {
            return execute_block(expr.else_()->body());
        }
        return value();
    }

    control_flow_expression_evaluator::result_type control_flow_expression_evaluator::operator()(ast::function_call_expression const& expr)
    {
        runtime::dispatcher dispatcher(_evaluator, expr.function().value(), expr.position());
        return dispatcher.dispatch(expr.arguments(), expr.lambda());
    }

    control_flow_expression_evaluator::result_type control_flow_expression_evaluator::execute_block(boost::optional<vector<ast::expression>> const& expressions)
    {
        value result;
        if (expressions) {
            for (size_t i = 0; i < expressions->size(); ++i) {
                auto& expression = (*expressions)[i];
                // The last expression in the block is allowed to be unproductive (i.e. the return value)
                result = _evaluator.evaluate(expression, i < (expressions->size() - 1));
            }
        }
        return result;
    }

}}}  // namespace puppet::runtime::evaluators