/**
 * @file
 * Declares the debug function.
 */
#pragma once

#include "descriptor.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    /**
     * Implements the debug function.
     */
    struct debug
    {
        /**
         * Create a function descriptor.
         * @return Returns the function descriptor representing this function.
         */
        static descriptor create_descriptor();
    };

}}}}  // puppet::compiler::evaluation::functions
