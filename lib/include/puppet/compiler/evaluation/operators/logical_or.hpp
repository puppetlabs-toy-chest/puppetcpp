/**
 * @file
 * Declares the logical 'or' operator.
 */
#pragma once

#include "binary_operator_context.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    /**
     * Implements the logical 'or' operator.
     */
    struct logical_or
    {
        /**
         * Called to invoke the operator.
         * @param context The binary operator context.
         * @return Returns the resulting value.
         */
        runtime::values::value operator()(binary_operator_context const& context) const;
    };

}}}}  // namespace puppet::compiler::evaluation::operators
