/**
 * @file
 * Declares the base fact provider.
 */
#pragma once

#include "../runtime/values/value.hpp"
#include <memory>
#include <string>
#include <functional>

namespace puppet { namespace facts {

    /**
     * Represents the base fact provider.
     */
    struct provider
    {
        /**
         * Looks up a fact value by name.
         * @param name The name of the fact to look up.
         * @return Returns the fact's value or nullptr if the fact is not found.
         */
        virtual std::shared_ptr<runtime::values::value const> lookup(std::string const& name) = 0;

        /**
         * Enumerates the facts in the provider.
         * @param accessed True to enumerate only the facts which have already been accessed or false to enumerate all facts.
         * @param callback The callback to call for each fact.
         */
        virtual void each(bool accessed, std::function<bool(std::string const&, std::shared_ptr<runtime::values::value const> const&)> const& callback) = 0;
    };

}}  // puppet::facts
