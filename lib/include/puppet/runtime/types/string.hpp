/**
 * @file
 * Declares the string type.
 */
#pragma once

#include "../values/forward.hpp"
#include <limits>
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of recursion_guard
    struct recursion_guard;

    // Forward declaration of integer
    struct integer;

    /**
     * Represents the Puppet String type.
     */
    struct string
    {
        /**
         * Constructs a string type.
         * @param from The "from" type parameter.
         * @param to The "to" type parameter.
         */
        explicit string(int64_t from = 0, int64_t to = std::numeric_limits<int64_t>::max());

        /**
         * Constructs a string from an integer range.
         * @param range The integer type describing the string's range.
         */
        explicit string(integer const& range);

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
         * @return Returns the name of the type (i.e. String).
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

        /**
         * Stores a default shared instance used internally by other Puppet types.
         */
        static string const instance;

     private:
        int64_t _from;
        int64_t _to;
    };

    /**
     * Stream insertion operator for string type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, string const& type);

    /**
     * Equality operator for string.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(string const& left, string const& right);

    /**
     * Inequality operator for string.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(string const& left, string const& right);

    /**
     * Hashes the string type.
     * @param type The string type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(string const& type);

}}}  // namespace puppet::runtime::types
