/**
 * @file
 * Declares the basic expression evaluator.
 */
#pragma once

#include "../expression_evaluator.hpp"

namespace puppet { namespace runtime { namespace evaluators {

    /**
     * Implements the basic expression evaluator.
     */
    struct basic_expression_evaluator : boost::static_visitor<values::value>
    {
        /**
         * Constructs a basic expression evaluator.
         * @param evaluator The expression evaluator to use.
         * @param expression The basic expression to evaluate.
         */
        basic_expression_evaluator(expression_evaluator& evaluator, ast::basic_expression const& expression);

        /**
         * Evaluates the basic expression.
         * @return Returns the evaluated value.
         */
        result_type evaluate();

     private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;
        result_type operator()(ast::undef const&);
        result_type operator()(ast::defaulted const&);
        result_type operator()(ast::boolean const& boolean);
        result_type operator()(int64_t integer);
        result_type operator()(long double floating);
        result_type operator()(ast::number const& number);
        result_type operator()(ast::string const& str);
        result_type operator()(ast::regex const& regx);
        result_type operator()(ast::variable const& var);
        result_type operator()(ast::name const& name);
        result_type operator()(ast::bare_word const& word);
        result_type operator()(ast::type const& type);
        result_type operator()(ast::array const& array);
        result_type operator()(ast::hash const& hash);

        expression_evaluator& _evaluator;
        ast::basic_expression const& _expression;
    };

}}}  // puppet::runtime::evaluators
