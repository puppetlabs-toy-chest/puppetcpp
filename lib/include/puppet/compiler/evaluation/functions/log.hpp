/**
 * @file
 * Declares the log functions.
 */
#pragma once

#include "function_call_context.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    /**
     * Implements the log functions (debug, info, notice, warning, err, etc.)
     */
    struct log
    {
        /**
         * Constructs a log function with the given log level.
         * @param level The log level to use when invoked.
         */
        explicit log(logging::level level);

        /**
         * Called to invoke the function.
         * @param context The function call context.
         * @return Returns the resulting value.
         */
        runtime::values::value operator()(function_call_context& context) const;

     private:
        logging::level _level;
    };

}}}}  // puppet::compiler::evaluation::functions
