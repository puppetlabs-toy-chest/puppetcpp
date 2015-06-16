/**
 * @file
 * Declares the include function.
 */
#pragma once

#include "../call_context.hpp"

namespace puppet { namespace runtime { namespace functions {

    /**
     * Implements the include function.
     */
    struct include
    {
        /**
         * Called to invoke the function.
         * @param context The function call context.
         * @return Returns the resulting value.
         */
        values::value operator()(call_context& context) const;
    };

}}}  // puppet::runtime::functions
