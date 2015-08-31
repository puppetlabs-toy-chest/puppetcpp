/**
 * @file
 * Declares the scalar type.
 */
#pragma once

#include "numeric.hpp"
#include "string.hpp"
#include "boolean.hpp"
#include "regexp.hpp"
#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Scalar type.
     */
    struct scalar
    {
        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Scalar).
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
            // A Scalar is a Numeric, String, Boolean, and Regexp
            return
                numeric().is_instance(value) ||
                string().is_instance(value) ||
                boolean().is_instance(value) ||
                regexp().is_instance(value);
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
            // Numeric, String, Boolean and Regexp are specializations
            // Also specializations of Numeric and String
            return boost::get<numeric>(&other) ||
                   boost::get<string>(&other) ||
                   boost::get<boolean>(&other) ||
                   boost::get<regexp>(&other) ||
                   numeric().is_specialization(other) ||
                   string().is_specialization(other);
        }
    };

    /**
     * Stream insertion operator for scalar type.
     * @param os The output stream to write the type to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, scalar const&);

    /**
     * Equality operator for scalar.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(scalar const&, scalar const&);

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Scalar type.
     */
    template <>
    struct hash<puppet::runtime::types::scalar>
    {
        /**
         * Hashes the Scalar type.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::scalar const&) const
        {
            static const size_t name_hash = boost::hash_value(puppet::runtime::types::scalar::name());

            size_t seed = 0;
            hash_combine(seed, name_hash);
            return seed;
        }
    };
}
