/**
 * @file
 * Declares the numeric type.
 */
#pragma once

#include "integer.hpp"
#include "floating.hpp"
#include <boost/functional/hash.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Numeric type.
     */
    struct numeric
    {
        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Numeric).
         */
        static const char* name();

        /**
         * Determines if the given value is an instance of this type.
         * @tparam Value The type of the runtime value.
         * @param value The value to determine if it is an instance of this type.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        template <typename Value>
        bool is_instance(Value const& value) const
        {
            return boost::get<int64_t>(&value) || boost::get<long double>(&value);
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
            // Integer or Float is a specialization
            return boost::get<integer>(&other) || boost::get<floating>(&other);
        }
    };

    /**
     * Stream insertion operator for numeric type.
     * @param os The output stream to write the type to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, numeric const&);

    /**
     * Equality operator for numeric.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(numeric const&, numeric const&);

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Numeric type.
     */
    template <>
    struct hash<puppet::runtime::types::numeric>
    {
        /**
         * Hashes the Numeric type.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::numeric const&) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::numeric::name());
            return seed;
        }
    };
}
