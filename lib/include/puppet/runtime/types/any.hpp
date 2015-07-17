/**
 * @file
 * Declares the any type.
 */
#pragma once

#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <ostream>
#include <functional>

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
            // All values are an instance of Any
            return true;
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
            // All types (except for Any) are specializations of Any
            return !boost::get<any>(&other);
        }
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
    bool operator==(any const&, any const&);

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Any type.
     */
    template <>
    struct hash<puppet::runtime::types::any>
    {
        /**
         * Hashes the Any type.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::any const&) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::any::name());
            return seed;
        }
    };
}
