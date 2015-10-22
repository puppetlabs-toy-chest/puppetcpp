/**
 * @file
 * Declares the "known list" collector.
 */
#pragma once

#include "collector.hpp"
#include "../../ast/ast.hpp"
#include "../../../runtime/types/resource.hpp"
#include <list>

namespace puppet { namespace compiler { namespace evaluation { namespace collectors {

    /**
     * Represents a collector that collects from a known list of resources.
     */
    struct list_collector : collector
    {
        /**
         * Constructs a list collector.
         * @param list The list of types to collect.
         */
        list_collector(std::list<std::pair<runtime::types::resource, ast::context>> list);

        /**
         * Collects the resources.
         * @param context The current evaluation context.
         */
        void collect(evaluation::context& context) override;

        /**
         * Detects uncollected resources.
         * Throws an evaluation exception if there are any uncollected resources.
         */
        void detect_uncollected() const override;

     private:
        std::list<std::pair<runtime::types::resource, ast::context>> _list;
    };

}}}}  // namespace puppet::compiler::evaluation::collectors
