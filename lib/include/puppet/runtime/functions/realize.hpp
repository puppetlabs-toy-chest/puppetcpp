/**
 * @file
 * Declares the realize function.
 */
#pragma once

#include "../call_context.hpp"

namespace puppet { namespace runtime { namespace functions {

    /**
     * Implements the "realize" function.
     */
    struct realize
    {
        /**
         * Called to invoke the function.
         * @param context The function call context.
         * @return Returns the resulting value.
         */
        values::value operator()(call_context& context) const;
    };

}}}  // puppet::runtime::functions
