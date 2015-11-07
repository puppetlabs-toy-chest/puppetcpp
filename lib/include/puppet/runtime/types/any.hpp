/**
 * @file
 * Declares the any type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Any type.
     */
    struct any
    {
        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Any).
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
     * Stream insertion operator for any type.
     * @param os The output stream to write the type to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, any const&);

    /**
     * Equality operator for any.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Always returns true (Any type is always equal to Any).
     */
    bool operator==(any const& left, any const& right);

    /**
     * Inequality operator for any.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Always returns false (Any type is always equal to Any).
     */
    bool operator!=(any const& left, any const& right);

    /**
     * Hashes the any type.
     * @param type The any type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(any const& type);

}}}  // namespace puppet::runtime::types
