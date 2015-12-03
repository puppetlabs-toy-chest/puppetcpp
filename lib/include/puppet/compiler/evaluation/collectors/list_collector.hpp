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
         * The type of the list used by the collector.
         */
        using list_type = std::list<std::pair<runtime::types::resource, ast::context>>;

        /**
         * Constructs a list collector.
         * @param list The list of types to collect.
         */
        explicit list_collector(list_type list);

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
        std::shared_ptr<ast::syntax_tree> _tree;
        list_type _list;
    };

}}}}  // namespace puppet::compiler::evaluation::collectors
