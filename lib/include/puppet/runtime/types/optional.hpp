/**
 * @file
 * Declares the optional type.
 */
#pragma once

#include "../values/undef.hpp"
#include "../../cast.hpp"
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Optional type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct basic_optional
    {
        /**
         * Constructs an Optional type.
         * @param type The optional type.
         */
        explicit basic_optional(boost::optional<Type> type) :
            _type(rvalue_cast(type))
        {
        }

        /**
         * Gets the optional type.
         * @return Returns the optional type.
         */
        boost::optional<Type> const& type() const
        {
            return _type;
        }

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Optional).
         */
        static const char* name()
        {
            return "Optional";
        }

        /**
         * Determines if the given value is an instance of this type.
         * @tparam Value The type of the runtime value.
         * @param value The value to determine if it is an instance of this type. This value will never be a variable.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        template <typename Value>
        bool is_instance(Value const& value) const
        {
            // Forward declaration of is_instance for recursion
            bool is_instance(Value const&, Type const&);

            // Undef always matches
            if (boost::get<values::undef>(&value)) {
                return true;
            }

            // Unparameterized Optional matches only undef
            if (!_type) {
                return false;
            }

            // Check that the instance is of the given type
            return is_instance(value, *_type);
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(Type const& other) const
        {
            // If this type has a specialization, the other type cannot be a specialization
            if (_type) {
                return false;
            }
            // Check that the other type is specialized
            auto optional = boost::get<basic_optional<Type>>(&other);
            return optional && optional->type();
        }

     private:
        boost::optional<Type> _type;
    };

    /**
     * Stream insertion operator for optional type.
     * @tparam Value The type of the runtime value.
     * @param os The output stream to write the type to.
     * @return Returns the given output stream.
     */
    template <typename Type>
    std::ostream& operator<<(std::ostream& os, basic_optional<Type> const& type)
    {
        os << basic_optional<Type>::name();
        if (!type.type()) {
            return os;
        }
        os << '[' << *type.type() << ']';
        return os;
    }

    /**
     * Equality operator for optional.
     * @tparam Value The type of the runtime value.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    template <typename Type>
    bool operator==(basic_optional<Type> const& left, basic_optional<Type> const& right)
    {
        return left.type() == right.type();
    }

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Optional type.
     * @tparam Value The type of the runtime value.
     */
    template <typename Type>
    struct hash<puppet::runtime::types::basic_optional<Type>>
    {
        /**
         * Hashes the Optional type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::basic_optional<Type> const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::basic_optional<Type>::name());
            if (type.type()) {
                hash_combine(seed, *type.type());
            }
            return seed;
        }
    };
}
