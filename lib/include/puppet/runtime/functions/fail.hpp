/**
 * @file
 * Declares the fail function.
 */
#pragma once

#include "../context.hpp"
#include "../value.hpp"
#include "../yielder.hpp"

namespace puppet { namespace runtime { namespace functions {

    /**
     * Implements the "fail" function.
     */
    struct fail
    {
        /**
         * Called to invoke the function.
         * @param ctx The current evaluation context.
         * @param position The position where the function call is.
         * @param arguments The arguments to the function.
         * @param yielder The yielder to use for invoking a lambda.
         * @return Returns the resulting value.
         */
        value operator()(context& ctx, lexer::token_position const& position, array& arguments, runtime::yielder& yielder) const;
    };

}}}  // puppet::runtime::functions
