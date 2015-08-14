/**
 * @file
 * Declares the relationship operators.
 */
#pragma once

#include "binary_context.hpp"

namespace puppet { namespace runtime { namespace operators {

    /**
     * Implements the in-edge (->) operator.
     */
    struct in_edge
    {
        /**
         * Called to invoke the operator.
         * @param context The operator context.
         * @return Returns the resulting value.
         */
        values::value operator()(binary_context& context) const;
    };

    /**
     * Implements the in-edge subscribe (~>) operator.
     */
    struct in_edge_subscribe
    {
        /**
         * Called to invoke the operator.
         * @param context The operator context.
         * @return Returns the resulting value.
         */
        values::value operator()(binary_context& context) const;
    };

    /**
     * Implements the out-edge (<-) operator.
     */
    struct out_edge
    {
        /**
         * Called to invoke the operator.
         * @param context The operator context.
         * @return Returns the resulting value.
         */
        values::value operator()(binary_context& context) const;
    };

    /**
     * Implements the out-edge subscribe (<~) operator.
     */
    struct out_edge_subscribe
    {
        /**
         * Called to invoke the operator.
         * @param context The operator context.
         * @return Returns the resulting value.
         */
        values::value operator()(binary_context& context) const;
    };

}}}  // puppet::runtime::operators
