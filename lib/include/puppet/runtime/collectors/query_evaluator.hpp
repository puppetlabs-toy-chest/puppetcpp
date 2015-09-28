/**
 * @file
 * Declares the query evaluator.
 */
#pragma once

#include "../../ast/collection_expression.hpp"
#include "../expression_evaluator.hpp"
#include "../catalog.hpp"

namespace puppet { namespace runtime { namespace collectors {

    /**
     * Represents a collection query evaluator.
     */
    struct query_evaluator
    {
        /**
         * Constructs a query evaluator given the query to evaluate.
         * @param evaluator The expression evaluator to use to evaluate query expressions.
         * @param query The optional query to evaluate.
         */
        query_evaluator(expression_evaluator& evaluator, boost::optional<ast::query> const& query);

        /**
         * Evaluates the query against the given resource.
         * @param resource The resource to evaluate the query against.
         * @return Returns true if the query evaluated to true for the resource or false if the query evaluated to false.
         */
        bool evaluate(runtime::resource const& resource) const;

     private:
        bool evaluate(ast::primary_attribute_query const& expression, runtime::resource const& resource) const;
        void climb_expression(
            bool& result,
            std::uint8_t min_precedence,
            std::vector<ast::binary_query_expression>::const_iterator& begin,
            std::vector<ast::binary_query_expression>::const_iterator const& end,
            runtime::resource const& resource) const;
        static uint8_t get_precedence(ast::binary_query_operator op);
        static bool is_right_associative(ast::binary_query_operator op);

        expression_evaluator& _evaluator;
        boost::optional<ast::query> const& _query;
    };

}}}  // namespace puppet::runtime:collectors
