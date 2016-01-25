/**
 * @file
 * Declares the splat operator.
 */
#pragma once

#include "descriptor.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace unary {

    /**
     * Implements the splat operator.
     */
    struct splat
    {
        /**
         * Create a unary operator descriptor.
         * @return Returns the unary operator descriptor representing this operator.
         */
        static descriptor create_descriptor();
    };

}}}}}  // namespace puppet::compiler::evaluation::operators::unary
