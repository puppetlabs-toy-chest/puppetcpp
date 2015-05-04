/**
 * @file
 * Declares the splat operator.
 */
#pragma once

#include "unary_context.hpp"

namespace puppet { namespace runtime { namespace operators {

    /**
     * Implements the splat operator.
     */
    struct splat
    {
        /**
         * Called to invoke the operator.
         * @param context The operator context.
         * @return Returns the resulting value.
         */
        values::value operator()(unary_context& context) const;
    };

}}}  // puppet::runtime::operators
