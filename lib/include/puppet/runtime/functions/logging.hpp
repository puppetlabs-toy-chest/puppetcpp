/**
 * @file
 * Declares the logging functions.
 */
#pragma once

#include "../values/value.hpp"
#include "../dispatcher.hpp"

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
         * @param context The function call context.
         * @return Returns the resulting value.
         */
        values::value operator()(call_context& context) const;

     private:
        logging::level _level;
    };

}}}  // puppet::runtime::functions
