/**
 * @file
 * Declares the type alias.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <unordered_map>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents a Puppet type alias.
     */
    struct alias
    {
        /**
         * Constructs a Puppet type alias.
         * @param name The type alias name.
         * @param resolved_type The type the alias resolved to.
         */
        alias(std::string name, std::shared_ptr<values::type> resolved_type);

        /**
         * Gets the name of the type alias.
         * @return Returns the name of the type alias.
         */
        std::string const& name() const;

        /**
         * Gets the resolved type of the alias.
         * @return Returns the resolved type of the alias.
         */
        values::type const& resolved_type() const;

        /**
         * Determines if the given value is an instance of this type.
         * @param value The value to determine if it is an instance of this type.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        bool is_instance(values::value const& value) const;

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(values::type const& other) const;

        /**
         * Determines if the type is real (i.e. actual type vs. an alias/variant that never resolves to an actual type).
         * @param map The map to keep track of encountered type aliases.
         * @return Returns true if the type is real or false if it never resolves to an actual type.
         */
        bool is_real(std::unordered_map<values::type const*, bool>& map) const;

        /**
         * Writes a representation of the type to the given stream.
         * @param stream The stream to write to.
         * @param expand True to specify that type aliases should be expanded or false if not.
         */
        void write(std::ostream& stream, bool expand = true) const;

     private:
        std::string _name;
        std::shared_ptr<values::type> _resolved_type;
    };

    /**
     * Stream insertion operator for alias type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, alias const& type);

    /**
     * Equality operator for alias.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Always returns true (Any type is always equal to Any).
     */
    bool operator==(alias const& left, alias const& right);

    /**
     * Inequality operator for alias.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Always returns false (Any type is always equal to Any).
     */
    bool operator!=(alias const& left, alias const& right);

    /**
     * Hashes the alias type.
     * @param type The alias type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(alias const& type);

}}}  // namespace puppet::runtime::types
