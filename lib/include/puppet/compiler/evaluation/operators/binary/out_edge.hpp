/**
 * @file
 * Declares the out-edge operator.
 */
#pragma once

#include "descriptor.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    /**
     * Implements the out-edge (<-) operator.
     */
    struct out_edge
    {
        /**
         * Create a binary operator descriptor.
         * @return Returns the binary operator descriptor representing this operator.
         */
        static descriptor create_descriptor();
    };

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
