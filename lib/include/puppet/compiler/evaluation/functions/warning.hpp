/**
 * @file
 * Declares the warning function.
 */
#pragma once

#include "descriptor.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    /**
     * Implements the warning function.
     */
    struct warning
    {
        /**
         * Create a function descriptor.
         * @return Returns the function descriptor representing this function.
         */
        static descriptor create_descriptor();
    };

}}}}  // puppet::compiler::evaluation::functions
