/**
 * @file
 * Declares the catalog entry type.
 */
#pragma once

#include "resource.hpp"
#include "class.hpp"
#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet CatalogEntry type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct basic_catalog_entry
    {
        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. CatalogEntry).
         */
        static const char* name()
        {
            return "CatalogEntry";
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
            return basic_resource<Type>().is_instance(value) ||
                   basic_class<Type>().is_instance(value);
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(Type const& other) const
        {
            // Resource and Class types are specializations
            return boost::get<basic_resource<Type>>(&other) ||
                   boost::get<basic_class<Type>>(&other);
        }
    };

    /**
     * Stream insertion operator for catalog entry type.
     * @tparam Type The type of a runtime type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    template <typename Type>
    std::ostream& operator<<(std::ostream& os, basic_catalog_entry<Type> const& type)
    {
        os << basic_catalog_entry<Type>::name();
        return os;
    }

    /**
     * Equality operator for catalog entry type.
     * @tparam Type The type of a runtime type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    template <typename Type>
    bool operator==(basic_catalog_entry<Type> const& left, basic_catalog_entry<Type> const& right)
    {
        return true;
    }

}}}  // puppet::runtime::types

namespace boost {
    /**
    * Hash specialization for CatalogEntry type.
    * @tparam Type The type of a runtime type.
    */
    template <typename Type>
    struct hash<puppet::runtime::types::basic_catalog_entry<Type>>
    {
        /**
         * Hashes the Class type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::basic_catalog_entry<Type> const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::basic_catalog_entry<Type>::name());
            return seed;
        }
    };
}
