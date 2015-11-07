/**
 * @file
 * Declares the less than or equal to operator.
 */
#pragma once

#include "binary_operator_context.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    /**
     * Implements the less than or equal to operator.
     */
    struct less_equal
    {
        /**
         * Called to invoke the operator.
         * @param context The binary operator context.
         * @return Returns the resulting value.
         */
        runtime::values::value operator()(binary_operator_context const& context) const;
    };

}}}}  // namespace puppet::compiler::evaluation::operators
