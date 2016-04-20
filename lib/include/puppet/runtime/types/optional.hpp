/**
 * @file
 * Declares the optional type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of recursion_guard
    struct recursion_guard;

    /**
     * Represents the Puppet Optional type.
     */
    struct optional
    {
        /**
         * Constructs an Optional type.
         * @param type The optional type.
         */
        explicit optional(std::unique_ptr<values::type> type = nullptr);

        /**
         * Copy constructor for optional type.
         * @param other The other optional type to copy from.
         */
        optional(optional const& other);

        /**
         * Move constructor for optional type.
         * Uses the default implementation.
         */
        optional(optional&&) noexcept = default;

        /**
         * Copy assignment operator for optional type.
         * @param other The other optional type to copy assign from.
         * @return Returns this optional type.
         */
        optional& operator=(optional const& other);

        /**
         * Move assignment operator for optional type.
         * Uses the default implementation.
         * @return Returns this optional type.
         */
        optional& operator=(optional&&) noexcept = default;

        /**
         * Gets the optional type.
         * @return Returns the optional type.
         */
        std::unique_ptr<values::type> const& type() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Optional).
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

     private:
        std::unique_ptr<values::type> _type;
    };

    /**
     * Stream insertion operator for optional type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, optional const& type);

    /**
     * Equality operator for optional.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(optional const& left, optional const& right);

    /**
     * Inequality operator for optional.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(optional const& left, optional const& right);

    /**
     * Hashes the optional type.
     * @param type The optional type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(optional const& type);

}}}  // namespace puppet::runtime::types
