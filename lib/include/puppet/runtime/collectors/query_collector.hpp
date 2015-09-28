/**
 * @file
 * Declares the query collector.
 */
#pragma once

#include "collector.hpp"
#include "../../compiler/context.hpp"
#include "../../ast/collection_expression.hpp"
#include "../scope.hpp"

namespace puppet { namespace runtime { namespace collectors {

    /**
     * Represents a query collector that collects resources based on a query.
     */
    struct query_collector : collector
    {
        /**
         * Constructs a query collector.
         * @param context The compilation context for the collector.
         * @param expression The collection expression to evaluate.
         * @param scope The scope the collector should operate in.
         */
        query_collector(std::shared_ptr<compiler::context> context, ast::collection_expression const& expression, std::shared_ptr<runtime::scope> scope);

        /**
         * Collects the resources.
         * @param context The current evaluation context.
         */
        void collect(runtime::context& context) override;

    private:
        std::shared_ptr<compiler::context> _context;
        ast::collection_expression const& _expression;
        std::shared_ptr<runtime::scope> _scope;
        size_t _index;
    };

}}}  // namespace puppet::runtime:collectors
