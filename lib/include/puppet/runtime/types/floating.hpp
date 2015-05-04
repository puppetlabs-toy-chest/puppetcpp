/**
 * @file
 * Declares the float type.
 */
#pragma once

#include <boost/functional/hash.hpp>
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
            auto ptr = boost::get<long double>(&value);
            if (!ptr) {
                return false;
            }
            return _to < _from ? (*ptr >= _to && *ptr <= _from) : (*ptr >= _from && *ptr <= _to);
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
            // Check for an Float with a range inside of this type's range
            auto ptr = boost::get<floating>(&other);
            if (!ptr) {
                return false;
            }
            // Check for equality
            if (ptr->from() == _from && ptr->to() == _to) {
                return false;
            }
            return std::min(ptr->from(), ptr->to()) >= std::min(_from, _to) &&
                   std::max(ptr->from(), ptr->to()) <= std::max(_from, _to);
        }

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

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Float type.
     */
    template <>
    struct hash<puppet::runtime::types::floating>
    {
        /**
         * Hashes the Float type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::floating const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::floating::name());
            hash_combine(seed, type.from());
            hash_combine(seed, type.to());
            return seed;
        }
    };
}
