/**
 * @file
 * Declares the resource type.
 */
#pragma once

#include <boost/functional/hash.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Resource type.
     */
    struct resource
    {
        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Resource).
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
     * Stream insertion operator for resource type.
     * @param os The output stream to write the type to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource const&);

    /**
     * Equality operator for resource.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(resource const&, resource const&);

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Resource type.
     */
    template <>
    struct hash<puppet::runtime::types::resource>
    {
        /**
         * Hashes the Resource type.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::resource const&) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::resource::name());
            return seed;
        }
    };
}
