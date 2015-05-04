/**
 * @file
 * Declares the logical "not" operator.
 */
#pragma once

#include "unary_context.hpp"

namespace puppet { namespace runtime { namespace operators {

    /**
     * Implements the logical "not" operator.
     */
    struct logical_not
    {
        /**
         * Called to invoke the operator.
         * @param context The operator context.
         * @return Returns the resulting value.
         */
        values::value operator()(unary_context& context) const;
    };

}}}  // puppet::runtime::operators
