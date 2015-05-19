/**
 * @file
 * Declares the control flow expression evaluator.
 */
#pragma once

#include "../expression_evaluator.hpp"

namespace puppet { namespace runtime { namespace evaluators {

    /**
     * Implements the control flow expression evaluator.
     */
    struct control_flow_expression_evaluator : boost::static_visitor<values::value>
    {
        /**
         * Constructs a control flow expression evaluator.
         * @param evaluator The expression evaluator to use.
         * @param expression The control flow expression to evaluate.
         */
        control_flow_expression_evaluator(expression_evaluator& evaluator, ast::control_flow_expression const& expression);

        /**
         * Evaluates the control flow expression.
         * @return Returns the evaluated value.
         */
        result_type evaluate();

     private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;
        result_type operator()(ast::case_expression const& expr);
        result_type operator()(ast::if_expression const& expr);
        result_type operator()(ast::unless_expression const& expr);
        result_type operator()(ast::function_call_expression const& expr);

        result_type execute_block(boost::optional<std::vector<ast::expression>> const& expressions);

        expression_evaluator& _evaluator;
        ast::control_flow_expression const& _expression;
    };

}}}  // puppet::runtime::evaluators
