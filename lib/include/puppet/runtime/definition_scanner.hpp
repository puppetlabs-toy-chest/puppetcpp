/**
 * @file
 * Declares the runtime definition scanner.
 */
#pragma once

#include <memory>

namespace puppet { namespace compiler {

    // Forward declaration of compiler context.
    struct context;

}}  // namespace puppet::compiler

namespace puppet { namespace runtime {

    // Forward declaration of catalog.
    struct catalog;

    /**
     * Represents the runtime definition scanner.
     * This type is responsible for scanning a syntax tree for catalog-related definitions.
     */
    struct definition_scanner
    {
        /**
         * Constructs a definition scanner.
         * @param catalog The catalog to populate with class, defined type, and node definitions.
         */
        explicit definition_scanner(runtime::catalog& catalog);

        /**
         * Scans the given compilation context's syntax tree for definitions.
         * @param context The compilation context to scan.
         */
        void scan(std::shared_ptr<compiler::context> const& context);

     private:
        runtime::catalog& _catalog;
    };

}}  // puppet::runtime
