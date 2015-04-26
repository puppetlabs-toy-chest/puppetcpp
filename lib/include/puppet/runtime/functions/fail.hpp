/**
 * @file
 * Declares the fail function.
 */
#pragma once

#include "../values/value.hpp"
#include "../dispatcher.hpp"

namespace puppet { namespace runtime { namespace functions {

    /**
     * Implements the "fail" function.
     */
    struct fail
    {
        /**
         * Called to invoke the function.
         * @param context The function call context.
         * @return Returns the resulting value.
         */
        values::value operator()(call_context& context) const;
    };

}}}  // puppet::runtime::functions
