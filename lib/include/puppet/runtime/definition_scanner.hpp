/**
 * @file
 * Declares the runtime definition scanner.
 */
#pragma once

#include "../ast/syntax_tree.hpp"
#include "context.hpp"
#include <memory>

namespace puppet { namespace runtime {

    /**
     * Represents the runtime definition scanner.
     * This type is responsible for scanning a syntax tree and
     * adding classes, defined types, and nodes to the evaluation context.
     */
    struct definition_scanner
    {
        /**
         * Constructs a definition scanner.
         * @param context The evaluation context.
         */
        explicit definition_scanner(runtime::context& context);

        /**
         * Scans the given compilation context's syntax tree for definition.
         * @param compilation_context The compilation context.
         */
        void scan(std::shared_ptr<compiler::context> compilation_context);

     private:
        runtime::context& _context;
    };

}}  // puppet::runtime
