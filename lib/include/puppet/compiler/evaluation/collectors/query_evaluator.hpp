/**
 * @file
 * Declares the query evaluator.
 */
#pragma once

#include "../../ast/ast.hpp"
#include "../../resource.hpp"
#include <boost/optional.hpp>

namespace puppet { namespace compiler { namespace evaluation {

    // Forward declaration of evaluation context
    struct context;

}}}  // namespace puppet::compiler::evaluation

namespace puppet { namespace compiler { namespace evaluation { namespace collectors {

    /**
     * Represents a collection query evaluator.
     */
    struct query_evaluator
    {
        /**
         * Constructs a query evaluator given the query to evaluate.
         * @param context The current evaluation context.
         * @param expression The query expression to evaluate.
         */
        query_evaluator(evaluation::context& context, boost::optional<ast::query_expression> const& expression);

        /**
         * Evaluates the query against the given resource.
         * @param resource The resource to evaluate the query against.
         * @return Returns true if the query evaluated to true for the resource or false if the query evaluated to false.
         */
        bool evaluate(compiler::resource const& resource) const;

     private:
        bool evaluate(ast::primary_query_expression const& expression, compiler::resource const& resource) const;
        bool climb_expression(
            ast::primary_query_expression const& expression,
            std::uint8_t min_precedence,
            std::vector<ast::binary_query_operation>::const_iterator& begin,
            std::vector<ast::binary_query_operation>::const_iterator const& end,
            compiler::resource const& resource) const;
        static uint8_t get_precedence(ast::binary_query_operator op);
        static bool is_right_associative(ast::binary_query_operator op);

        evaluation::context& _context;
        boost::optional<ast::query_expression> const& _expression;
    };

}}}}  // namespace puppet::compiler::evaluation::collectors
