/**
 * @file
 * Declares the class declaration functions (e.g. include, contain, and require).
 */
#pragma once

#include "../call_context.hpp"
#include <boost/optional.hpp>

namespace puppet { namespace runtime { namespace functions {

    /**
     * Implements a function related to class loading
     */
    struct declare
    {
        /**
         * Constructs the klass function with the optional relationship.
         * @param relationship The relationship the class should form after beinge evaluated.
         */
        explicit declare(boost::optional<runtime::relationship> relationship = boost::none);

        /**
         * Called to invoke the function.
         * @param context The function call context.
         * @return Returns the resulting value.
         */
        values::value operator()(call_context& context) const;

     private:
        boost::optional<runtime::relationship> _relationship;
    };

}}}  // puppet::runtime::functions
