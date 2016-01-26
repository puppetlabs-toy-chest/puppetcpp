/**
 * @file
 * Declares the logical 'not' operator.
 */
#pragma once

#include "descriptor.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace unary {

    /**
     * Implements the logical 'not' operator.
     */
    struct logical_not
    {
        /**
         * Create a unary operator descriptor.
         * @return Returns the unary operator descriptor representing this operator.
         */
        static descriptor create_descriptor();
    };

}}}}}  // namespace puppet::compiler::evaluation::operators::unary
