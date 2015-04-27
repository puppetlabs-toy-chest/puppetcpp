/**
 * @file
 * Declares the assert_type function.
 */
#pragma once

#include "../values/value.hpp"
#include "../dispatcher.hpp"

namespace puppet { namespace runtime { namespace functions {

    /**
     * Implements the assert_type function.
     */
    struct assert_type
    {
        /**
         * Called to invoke the function.
         * @param context The function call context.
         * @return Returns the resulting value.
         */
        values::value operator()(call_context& context) const;
    };

}}}  // puppet::runtime::functions
