/**
 * @file
 * Declares the logging functions.
 */
#pragma once

#include "../context.hpp"
#include "../value.hpp"
#include "../yielder.hpp"

namespace puppet { namespace runtime { namespace functions {

    /**
     * Implements the logging functions (debug, info, notice, warning, err, etc.)
     */
    struct logging_function
    {
        /**
         * Constructs a logging function with the given log level.
         * @param lvl The log level to use when invoked.
         */
        explicit logging_function(logging::level lvl);

        /**
         * Called to invoke the function.
         * @param ctx The current evaluation context.
         * @param position The position where the function call is.
         * @param arguments The arguments to the function.
         * @param yielder The yielder to use for invoking a lambda.
         * @return Returns the resulting value.
         */
        value operator()(context& ctx, lexer::token_position const& position, array& arguments, runtime::yielder& yielder) const;

     private:
        logging::level _level;
    };

}}}  // puppet::runtime::functions
