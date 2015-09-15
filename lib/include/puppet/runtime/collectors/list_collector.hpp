/**
 * @file
 * Declares the "known list" collector.
 */
#pragma once

#include "collector.hpp"
#include "../values/type.hpp"
#include <list>

namespace puppet { namespace runtime { namespace collectors {

    /**
     * Represents a collector that collects from a known list of resources.
     */
    struct list_collector : collector
    {
        /**
         * Constructs a list collector.
         * @param context The compilation context where the list was specified.
         * @param list The list of types to collect.
         */
        list_collector(std::shared_ptr<compiler::context> context, std::list<std::pair<types::resource, lexer::position>> list);

        /**
         * Collects the resources.
         * @param catalog The catalog to collect resources from.
         */
        void collect(runtime::catalog& catalog) override;

        /**
         * Detects uncollected resources.
         * Throws an evaluation exception if there are any uncollected resources.
         */
        void detect_uncollected() const override;

     private:
        std::shared_ptr<compiler::context> _context;
        std::list<std::pair<types::resource, lexer::position>> _list;
    };

}}}  // namespace puppet::runtime:collectors
