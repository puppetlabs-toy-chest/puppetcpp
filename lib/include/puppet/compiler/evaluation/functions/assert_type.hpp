/**
 * @file
 * Declares the assert_type function.
 */
#pragma once

#include "function_call_context.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

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
        runtime::values::value operator()(function_call_context& context) const;

     private:
        static bool validate_type(function_call_context& context, runtime::values::type const& type, runtime::values::value& instance, ast::context const& argument_context);
    };

}}}}  // puppet::compiler::evaluation::functions
