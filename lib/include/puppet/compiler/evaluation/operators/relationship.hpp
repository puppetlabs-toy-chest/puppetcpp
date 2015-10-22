/**
 * @file
 * Declares the relationship operators.
 */
#pragma once

#include "binary_operator_context.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    /**
     * Implements the in-edge (->) operator.
     */
    struct in_edge
    {
        /**
         * Called to invoke the operator.
         * @param context The binary operator context.
         * @return Returns the resulting value.
         */
        runtime::values::value operator()(binary_operator_context const& context) const;
    };

    /**
     * Implements the in-edge subscribe (~>) operator.
     */
    struct in_edge_subscribe
    {
        /**
         * Called to invoke the operator.
         * @param context The binary operator context.
         * @return Returns the resulting value.
         */
        runtime::values::value operator()(binary_operator_context const& context) const;
    };

    /**
     * Implements the out-edge (<-) operator.
     */
    struct out_edge
    {
        /**
         * Called to invoke the operator.
         * @param context The binary operator context.
         * @return Returns the resulting value.
         */
        runtime::values::value operator()(binary_operator_context const& context) const;
    };

    /**
     * Implements the out-edge subscribe (<~) operator.
     */
    struct out_edge_subscribe
    {
        /**
         * Called to invoke the operator.
         * @param context The binary operator context.
         * @return Returns the resulting value.
         */
        runtime::values::value operator()(binary_operator_context const& context) const;
    };


}}}}  // namespace puppet::compiler::evaluation::operators
