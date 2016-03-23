/**
 * @file
 * Declares the query collector.
 */
#pragma once

#include "collector.hpp"
#include "../scope.hpp"
#include "../../ast/ast.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace collectors {

    /**
     * Represents a query collector that collects resources based on a query.
     */
    struct query_collector : collector
    {
        /**
         * Constructs a query collector.
         * @param expression The collection expression to evaluate.
         * @param scope The scope the collector should operate in.
         */
        query_collector(ast::collector_expression const& expression, std::shared_ptr<evaluation::scope> scope);

        /**
         * Collects the resources.
         * @param context The current evaluation context.
         */
        void collect(evaluation::context& context) override;

     private:
        ast::collector_expression const& _expression;
        std::shared_ptr<scope> _scope;
        size_t _index;
    };

}}}}  // namespace puppet::compiler::evaluation::collectors
