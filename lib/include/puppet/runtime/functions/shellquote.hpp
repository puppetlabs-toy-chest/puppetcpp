/**
 * @file
 * Declares the shellquote function.
 */
#pragma once

#include "../call_context.hpp"

namespace puppet { namespace runtime { namespace functions {

    /**
     * Implements the shellquote function.
     */
    struct shellquote
    {
        /**
         * Called to invoke the function.
         * @param context The function call context.
         * @return Returns the resulting value.
         */
        values::value operator()(call_context& context) const;

        private:
            size_t _count_chars(const std::string &word, const std::string &set) const;
    };

}}}  // puppet::runtime::functions
