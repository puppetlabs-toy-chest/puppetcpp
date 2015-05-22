/**
 * @file
 * Declares the postfix expression evaluator.
 */
#pragma once

#include "../expression_evaluator.hpp"

namespace puppet { namespace runtime { namespace evaluators {

    /**
     * Implements the postfix expression evaluator.
     */
    struct postfix_expression_evaluator : boost::static_visitor<values::value>
    {
        /**
         * Constructs a postfix expression evaluator.
         * @param evaluator The expression evaluator to use.
         * @param expression The postfix expression to evaluate.
         */
        postfix_expression_evaluator(expression_evaluator& evaluator, ast::postfix_expression const& expression);

        /**
         * Evaluates the postfix expression.
         * @return Returns the evaluated value.
         */
        result_type evaluate();

     private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;
        result_type operator()(ast::selector_expression const& expr);
        result_type operator()(ast::access_expression const& expr);
        result_type operator()(ast::method_call_expression const& expr);

        expression_evaluator& _evaluator;
        ast::postfix_expression const& _expression;
        values::value _result;
        lexer::position _position;
        ast::primary_expression const* _first_expression;
    };

}}}  // puppet::runtime::evaluators
