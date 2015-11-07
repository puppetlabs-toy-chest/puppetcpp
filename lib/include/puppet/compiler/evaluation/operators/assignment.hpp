/**
 * @file
 * Declares the assignment operator.
 */
#pragma once

#include "binary_operator_context.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    /**
     * Implements the assignment operator.
     */
    struct assignment
    {
        /**
         * Called to invoke the operator.
         * @param context The binary operator context.
         * @return Returns the resulting value.
         */
        runtime::values::value operator()(binary_operator_context const& context) const;
    };

}}}}  // namespace puppet::compiler::evaluation::operators
