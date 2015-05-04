/**
 * @file
 * Declares the logical "and" operator.
 */
#pragma once

#include "binary_context.hpp"

namespace puppet { namespace runtime { namespace operators {

    /**
     * Implements the logical "and" operator.
     */
    struct logical_and
    {
        /**
         * Called to invoke the operator.
         * @param context The operator context.
         * @return Returns the resulting value.
         */
        values::value operator()(binary_context& context) const;
    };

}}}  // puppet::runtime::operators
