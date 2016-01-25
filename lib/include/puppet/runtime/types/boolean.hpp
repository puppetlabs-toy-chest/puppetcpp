/**
 * @file
 * Declares the boolean type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Boolean type.
     */
    struct boolean
    {
        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Boolean).
         */
        static char const* name();

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
    };

    /**
     * Stream insertion operator for boolean type.
     * @param os The output stream to write the type to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, boolean const&);

    /**
     * Equality operator for boolean.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(boolean const& left, boolean const& right);

    /**
     * Inequality operator for boolean.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(boolean const& left, boolean const& right);

    /**
     * Hashes the boolean type.
     * @param type The boolean type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(boolean const& type);

}}}  // namespace puppet::runtime::types
