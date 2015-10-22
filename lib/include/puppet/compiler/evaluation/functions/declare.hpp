/**
 * @file
 * Declares the class declaration functions (e.g. include, contain, and require).
 */
#pragma once

#include "function_call_context.hpp"
#include <boost/optional.hpp>

namespace puppet { namespace compiler {

    // Forward declaration of relationship enum.
    enum class relationship;

}}  // namespace puppet::compiler

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    /**
     * Implements the declare function.
     */
    struct declare
    {
        /**
         * Constructs the declare function with the optional relationship.
         * @param relationship The relationship the class should form with the container after being declared.
         */
        explicit declare(boost::optional<compiler::relationship> relationship = boost::none);

        /**
         * Called to invoke the function.
         * @param context The function call context.
         * @return Returns the resulting value.
         */
        runtime::values::value operator()(function_call_context& context) const;

     private:
        boost::optional<compiler::relationship> _relationship;
    };

}}}}  // puppet::compiler::evaluation::functions
