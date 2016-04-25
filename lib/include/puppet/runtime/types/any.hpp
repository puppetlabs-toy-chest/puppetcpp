/**
 * @file
 * Declares the any type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of recursion_guard
    struct recursion_guard;

    /**
     * Represents the Puppet Any type.
     */
    struct any
    {
        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Any).
         */
        static char const* name();

        /**
         * Creates a generalized version of the type.
         * @return Returns the generalized type.
         */
        values::type generalize() const;

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
    };

    /**
     * Stream insertion operator for any type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, any const& type);

    /**
     * Equality operator for any.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Always returns true (Any type is always equal to Any).
     */
    bool operator==(any const& left, any const& right);

    /**
     * Inequality operator for any.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Always returns false (Any type is always equal to Any).
     */
    bool operator!=(any const& left, any const& right);

    /**
     * Hashes the any type.
     * @param type The any type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(any const& type);

}}}  // namespace puppet::runtime::types
