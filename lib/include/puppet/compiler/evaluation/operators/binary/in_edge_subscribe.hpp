/**
 * @file
 * Declares the in-edge-subscribe operator.
 */
#pragma once

#include "descriptor.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    /**
     * Implements the in-edge-subscribe (~>) operator.
     */
    struct in_edge_subscribe
    {
        /**
         * Create a binary operator descriptor.
         * @return Returns the binary operator descriptor representing this operator.
         */
        static descriptor create_descriptor();
    };

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
