/**
 * @file
 * Declares the collection type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <limits>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Collection type.
     */
    struct collection
    {
        /**
         * Constructs a collection type.
         * @param from The "from" type parameter.
         * @param to The "to" type parameter.
         */
        explicit collection(int64_t from = std::numeric_limits<int64_t>::min(), int64_t to = std::numeric_limits<int64_t>::max());

        /**
         * Gets the "from" type parameter.
         * @return Returns the "from" type parameter.
         */
        int64_t from() const;

        /**
         * Gets the "to" type parameter.
         * @return Returns the "to" type parameter.
         */
        int64_t to() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Collection).
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
        int64_t _from;
        int64_t _to;
    };

    /**
     * Stream insertion operator for collection type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, collection const& type);

    /**
     * Equality operator for collection.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(collection const& left, collection const& right);

    /**
     * Inequality operator for collection.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(collection const& left, collection const& right);

    /**
     * Hashes the collection type.
     * @param type The collection type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(collection const& type);

}}}  // namespace puppet::runtime::types

