/**
 * @file
 * Declares the "type" type.
 */
#pragma once

#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Type type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct basic_type
    {
        /**
         * Constructs an Type type.
         * @param type The optional type.
         */
        explicit basic_type(boost::optional<Type> type) :
            _type(std::move(type))
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
         * @return Returns the name of the type (i.e. Type).
         */
        static const char* name()
        {
            return "Type";
        }

        /**
         * Determines if the given value is an instance of this type.
         * @tparam Value The type of the runtime value.
         * @param value The value to determine if it is an instance of this type.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        template <typename Value>
        bool is_instance(Value const& value) const
        {
            // Check for type
            auto ptr = boost::get<Type>(&value);
            if (!ptr) {
                return false;
            }
            // Unparameterized Type matches all types
            if (!_type) {
                return true;
            }
            // Compare the types
            return *ptr == *_type;
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(Type const& other) const
        {
            // If this Type has a specialization, the other type cannot be a specialization
            if (_type) {
                return false;
            }
            // Check that the other Type is specialized
            auto type = boost::get<basic_type<Type>>(&other);
            return type && type->type();
        }

     private:
        boost::optional<Type> _type;
    };

    /**
     * Stream insertion operator for "type" type.
     * @tparam Type The type of a runtime type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    template <typename Type>
    std::ostream& operator<<(std::ostream& os, basic_type<Type> const& type)
    {
        os << basic_type<Type>::name();
        if (!type.type()) {
            return os;
        }
        os << '[' << *type.type() << ']';
        return os;
    }

    /**
     * Equality operator for type.
     * @tparam Type The type of a runtime type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    template <typename Type>
    bool operator==(basic_type<Type> const& left, basic_type<Type> const& right)
    {
        return left.type() == right.type();
    }

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Type type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct hash<puppet::runtime::types::basic_type<Type>>
    {
        /**
         * Hashes the Type type.
         * @param type The type to hash.
         * @return Returns the type value for the type.
         */
        size_t operator()(puppet::runtime::types::basic_type<Type> const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::basic_type<Type>::name());
            if (type.type()) {
                hash_combine(seed, *type.type());
            }
            return seed;
        }
    };
}
