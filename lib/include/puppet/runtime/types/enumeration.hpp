/**
 * @file
 * Declares the enumeration type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <vector>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of recursion_guard
    struct recursion_guard;

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
        static char const* name();

        /**
         * Determines if the given value is an instance of this type.
         * @param value The value to determine if it is an instance of this type.
         * @param guard The recursion guard to use for aliases.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        bool is_instance(values::value const& value, recursion_guard& guard) const;

        /**
         * Determines if the given type is assignable to this type.
         * @param other The other type to check for assignability.
         * @param guard The recursion guard to use for aliases.
         * @return Returns true if the given type is assignable to this type or false if the given type is not assignable to this type.
         */
        bool is_assignable(values::type const& other, recursion_guard& guard) const;

        /**
         * Writes a representation of the type to the given stream.
         * @param stream The stream to write to.
         * @param expand True to specify that type aliases should be expanded or false if not.
         */
        void write(std::ostream& stream, bool expand = true) const;

     private:
        std::vector<std::string> _strings;
    };

    /**
     * Stream insertion operator for enumeration type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, enumeration const& type);

    /**
     * Equality operator for enumeration.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(enumeration const& left, enumeration const& right);

    /**
     * Inequality operator for enumeration.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(enumeration const& left, enumeration const& right);

    /**
     * Hashes the enumeration type.
     * @param type The enumeration type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(enumeration const& type);

}}}  // namespace puppet::runtime::types

