/**
 * @file
 * Declares the "data" type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Data type.
     */
    struct data
    {
        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Data).
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
     * Stream insertion operator for data type.
     * @param os The output stream to write the type to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, data const&);

    /**
     * Equality operator for data.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(data const& left, data const& right);

    /**
     * Inequality operator for collection.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(data const& left, data const& right);

    /**
     * Hashes the data type.
     * @param type The data type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(data const& type);

}}}  // namespace puppet::runtime::types
