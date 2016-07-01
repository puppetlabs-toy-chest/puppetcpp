/**
 * @file
 * Declares the epp function.
 */
#pragma once

#include "descriptor.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    /**
     * Implements the epp function.
     */
    struct epp
    {
        /**
         * Create a function descriptor.
         * @return Returns the function descriptor representing this function.
         */
        static descriptor create_descriptor();
    };

}}}}  // puppet::compiler::evaluation::functions
