/**
 * @file
 * Declares the runtime definition scanner.
 */
#pragma once

#include "../ast/syntax_tree.hpp"
#include "catalog.hpp"
#include <memory>

namespace puppet { namespace runtime {

    /**
     * Represents the runtime definition scanner.
     * This type is responsible for scanning a syntax tree and
     * defining classes, defined types, and nodes in a catalog.
     */
    struct definition_scanner
    {
        /**
         * Constructs a definition scanner.
         * @param catalog The catalog to populate with definitions.
         */
        explicit definition_scanner(runtime::catalog& catalog);

        /**
         * Scans the given compilation context's syntax tree for definition.
         * @param compilation_context The compilation context.
         */
        void scan(std::shared_ptr<compiler::context> compilation_context);

     private:
        runtime::catalog& _catalog;
    };

}}  // puppet::runtime
