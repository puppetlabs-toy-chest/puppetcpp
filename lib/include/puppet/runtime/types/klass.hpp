/**
 * @file
 * Declares the class type.
 */
#pragma once

#include <boost/functional/hash.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Class type.
     */
    struct klass
    {
        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Class).
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
            // TODO: implement
            return false;
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
            // TODO: implement
            return false;
        }
    };

    /**
     * Stream insertion operator for class type.
     * @param os The output stream to write the type to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, klass const&);

    /**
     * Equality operator for boolean.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(klass const&, klass const&);

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Class type.
     */
    template <>
    struct hash<puppet::runtime::types::klass>
    {
        /**
         * Hashes the Class type.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::klass const&) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::klass::name());
            return seed;
        }
    };
}
