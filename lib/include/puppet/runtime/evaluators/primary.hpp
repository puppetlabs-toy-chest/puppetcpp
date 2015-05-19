/**
 * @file
 * Declares the primary expression evaluator.
 */
#pragma once

#include "../expression_evaluator.hpp"

namespace puppet { namespace runtime { namespace evaluators {

    /**
     * Implements the primary expression evaluator.
     */
    struct primary_expression_evaluator : boost::static_visitor<values::value>
    {
        /**
         * Constructs a primary expression evaluator.
         * @param evaluator The expression evaluator to use.
         * @param expression The primary expression to evaluate.
         */
        primary_expression_evaluator(expression_evaluator& evaluator, ast::primary_expression const& expression);

        /**
         * Evaluates the primary expression.
         * @return Returns the evaluated value.
         */
        result_type evaluate();

    private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;
        result_type operator()(boost::blank const&);
        result_type operator()(ast::basic_expression const& expr);
        result_type operator()(ast::control_flow_expression const& expr);
        result_type operator()(ast::catalog_expression const& expr);
        result_type operator()(ast::unary_expression const& expr);
        result_type operator()(ast::postfix_expression const& expr);
        result_type operator()(ast::expression const& expr);

        expression_evaluator& _evaluator;
        ast::primary_expression const& _expression;
    };

}}}  // puppet::runtime::evaluators
