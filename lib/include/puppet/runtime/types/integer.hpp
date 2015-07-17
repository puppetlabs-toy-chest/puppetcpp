/**
 * @file
 * Declares the integer type.
 */
#pragma once

#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <limits>
#include <cstdint>
#include <ostream>
#include <functional>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Integer[from, to] type.
     */
    struct integer
    {
        /**
         * Constructs an integer type.
         * @param from The "from" type parameter.
         * @param to The "to" type parameter.
         */
        explicit integer(int64_t from = std::numeric_limits<int64_t>::min(), int64_t to = std::numeric_limits<int64_t>::max());

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
         * @return Returns the name of the type (i.e. Integer).
         */
        static const char* name();

        /**
         * Determines if the range is enumerable.
         * @return Returns whether or not the range is enumerable.
         */
        bool enumerable() const;

        /**
         * Calls the given callback for each integer in the range.
         * @param callback The callback to call.
         */
        void each(std::function<bool(int64_t, int64_t)> const& callback) const;

        /**
         * Determines if the given value is an instance of this type.
         * @tparam Value The type of the runtime value.
         * @param value The value to determine if it is an instance of this type. This value will never be a variable.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        template <typename Value>
        bool is_instance(Value const& value) const
        {
            auto ptr = boost::get<int64_t>(&value);
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
            // Check for an Integer with a range inside of this type's range
            auto ptr = boost::get<integer>(&other);
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
        int64_t _from;
        int64_t _to;
    };

    /**
     * Stream insertion operator for integer type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, integer const& type);

    /**
     * Equality operator for integer.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(integer const& left, integer const& right);

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Integer type.
     */
    template <>
    struct hash<puppet::runtime::types::integer>
    {
        /**
         * Hashes the Integer type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::integer const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::integer::name());
            hash_combine(seed, type.from());
            hash_combine(seed, type.to());
            return seed;
        }
    };
}
