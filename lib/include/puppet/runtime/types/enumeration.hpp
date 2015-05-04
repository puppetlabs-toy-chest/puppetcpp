/**
 * @file
 * Declares the enumeration type.
 */
#pragma once

#include <boost/functional/hash.hpp>
#include <ostream>
#include <vector>
#include <string>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Enum type.
     */
    struct enumeration
    {
        /**
         * Constructs an Enum type.
         * @param strings The strings that make up the enumeration.
         */
        explicit enumeration(std::vector<std::string> strings = std::vector<std::string>());

        /**
         * Gets the strings of the enumeration.
         * @return Returns the strings of the enumeration.
         */
        std::vector<std::string> const& strings() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Enum).
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
            auto ptr = boost::get<std::string>(&value);
            if (!ptr) {
                return false;
            }
            if (_strings.empty()) {
                return true;
            }
            for (auto const& string : _strings) {
                if (string == *ptr) {
                    return true;
                }
            }
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
            // Specializations of Enum have *fewer* strings (i.e. are more restrictive)
            auto ptr = boost::get<enumeration>(&other);
            if (!ptr) {
                return false;
            }
            return ptr->strings().size() < _strings.size();
        }

     private:
        std::vector<std::string> _strings;
    };

    /**
     * Stream insertion operator for enum type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, enumeration const& type);

    /**
     * Equality operator for enum.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(enumeration const& left, enumeration const& right);

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Enum type.
     */
    template <>
    struct hash<puppet::runtime::types::enumeration>
    {
        /**
         * Hashes the Enum type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::enumeration const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::enumeration::name());
            hash_range(seed, type.strings().begin(), type.strings().end());
            return seed;
        }
    };
}
