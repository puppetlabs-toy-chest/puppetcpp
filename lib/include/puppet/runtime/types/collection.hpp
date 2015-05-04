/**
 * @file
 * Declares the collection type.
 */
#pragma once

#include "array.hpp"
#include "hash.hpp"
#include "tuple.hpp"
#include "struct.hpp"
#include "../values/array.hpp"
#include "../values/hash.hpp"
#include <boost/functional/hash.hpp>
#include <ostream>

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
        static const char* name();

        /**
         * Determines if the given value is an instance of this type.
         * @tparam Value The type of the runtime value.
         * @param value The value to determine if it is an instance of this type. This value will never be a variable.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        template <typename Value>
        bool is_instance(Value const& value) const
        {
            // Check for array first
            int64_t size = 0;
            auto array = boost::get<values::basic_array<Value>>(&value);
            if (array) {
                size = static_cast<int64_t>(array->size());
            } else {
                // Check for hash
                auto hash = boost::get<values::basic_hash<Value>>(&value);
                if (hash) {
                    size = static_cast<int64_t>(hash->size());
                } else {
                    // Not a collection
                    return false;
                }
            }
            // Check for size is range
            return _to < _from ? (size >= _to && size <= _from) : (size >= _from && size <= _to);
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @tparam Type The type of runtime type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        template <typename Type>
        bool is_specialization(Type const& other) const
        {
            // Array and Hash are specializations
            // So are any specializations of Array and Hash
            return boost::get<basic_array<Type>>(&other)        ||
                   boost::get<basic_hash<Type>>(&other)         ||
                   basic_array<Type>().is_specialization(other) ||
                   basic_hash<Type>().is_specialization(other);
        }

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

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Collection type.
     */
    template <>
    struct hash<puppet::runtime::types::collection>
    {
        /**
         * Hashes the Collection type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::collection const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::collection::name());
            hash_combine(seed, type.from());
            hash_combine(seed, type.to());
            return seed;
        }
    };
}
