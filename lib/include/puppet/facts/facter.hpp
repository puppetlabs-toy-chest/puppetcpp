/**
 * @file
 * Declares the facter fact provider.
 */
#pragma once

#include "provider.hpp"
#include "../logging/logger.hpp"
#include <unordered_map>

namespace facter { namespace facts {

    // Forward declaration of collection
    struct collection;

    // Forward declaration of value
    struct value;

}}  // namespace facter::facts

namespace puppet { namespace facts {

    /**
     * Represents the facter fact provider.
     */
    struct facter : provider
    {
        /**
         * Constructs a new facter fact provider.
         */
        facter();

        /**
         * Looks up a fact value by name.
         * @param name The name of the fact to look up.
         * @return Returns the fact's value or nullptr if the fact is not found.
         */
        std::shared_ptr<runtime::values::value const> lookup(std::string const& name) override;

        /**
         * Enumerates the facts in the provider.
         * @param accessed True to enumerate only the facts which have already been accessed or false to enumerate all facts.
         * @param callback The callback to call for each fact.
         */
        void each(bool accessed, std::function<bool(std::string const&, std::shared_ptr<runtime::values::value const> const&)> const& callback) override;

     private:
        void store(std::string const& name, ::facter::facts::value const* value, runtime::values::value* parent = nullptr);

        ::facter::facts::collection _collection;
        std::unordered_map<std::string, std::shared_ptr<runtime::values::value const>> _cache;
    };

}}  // puppet::facts
