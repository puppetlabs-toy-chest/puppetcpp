/**
 * @file
 * Declares the integer type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <limits>
#include <cstdint>
#include <functional>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of recursion_guard
    struct recursion_guard;

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
        static char const* name();

        /**
         * Determines if the range is iterable.
         * @return Returns whether or not the range is iterable.
         */
        bool iterable() const;

        /**
         * Calls the given callback for each integer in the range.
         * @param callback The callback to call.
         */
        void each(std::function<bool(int64_t, int64_t)> const& callback) const;

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

        /**
         * Instantiates a new instance of the type.
         * @param from The value to convert from.
         * @param radix The radix to use (0 means 'detect from string prefix'). This parameter is only used for conversions from string.
         * @return Returns the instantiated value.
         */
        static values::value instantiate(values::value from, int radix = 0);

        /**
         * Stores a default shared instance used internally by other Puppet types.
         */
        static integer const instance;

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

    /**
     * Inequality operator for integer.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(integer const& left, integer const& right);

    /**
     * Hashes the integer type.
     * @param type The integer type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(integer const& type);

}}}  // namespace puppet::runtime::types
