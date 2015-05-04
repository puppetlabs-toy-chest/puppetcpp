/**
 * @file
 * Declares the right shift operator.
 */
#pragma once

#include "binary_context.hpp"

namespace puppet { namespace runtime { namespace operators {

    /**
     * Implements the right shift operator.
     */
    struct right_shift
    {
        /**
         * Called to invoke the operator.
         * @param context The operator context.
         * @return Returns the resulting value.
         */
        values::value operator()(binary_context& context) const;
    };

}}}  // puppet::runtime::operators
