/**
 * @file
 * Declares the float type.
 */
#pragma once

#include "../values/forward.hpp"
#include <limits>
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Float type.
     */
    struct floating
    {
        /**
         * Constructs a float type.
         * @param from The "from" type parameter.
         * @param to The "to" type parameter.
         */
        explicit floating(long double from = std::numeric_limits<long double>::min(), long double to = std::numeric_limits<long double>::max());

        /**
         * Gets the "from" type parameter.
         * @return Returns the "from" type parameter.
         */
        long double from() const;

        /**
         * Gets the "to" type parameter.
         * @return Returns the "to" type parameter.
         */
        long double to() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Float).
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

     private:
        long double _from;
        long double _to;
    };

    /**
     * Stream insertion operator for float type.
     * @param os The output stream to write the type to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, floating const& type);

    /**
     * Equality operator for float.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(floating const& left, floating const& right);

    /**
     * Inequality operator for float.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
     bool operator!=(floating const& left, floating const& right);

    /**
     * Hashes the float type.
     * @param type The float type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(floating const& type);

}}}  // namespace puppet::runtime::types
