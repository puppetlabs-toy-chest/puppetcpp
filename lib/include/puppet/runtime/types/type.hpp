/**
 * @file
 * Declares the "type" type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of recursion_guard
    struct recursion_guard;

    /**
     * Represents the Puppet Type type.
     */
    struct type
    {
        /**
         * Constructs an Type type.
         * @param parameter The optional type parameter.
         */
        explicit type(std::unique_ptr<values::type const> parameter = nullptr);

        /**
         * Copy constructor for 'type' type.
         * @param other The other 'type' type to copy from.
         */
        type(type const& other);

        /**
         * Move constructor for 'type' type.
         * Uses the default implementation.
         */
        type(type&&) noexcept = default;

        /**
         * Copy assignment operator for 'type' type.
         * @param other The other type 'type' to copy assign from.
         * @return Returns this 'type' type.
         */
        type& operator=(type const& other);

        /**
         * Move assignment operator for 'type' type.
         * Uses the default implementation.
         * @return Returns this 'type' type.
         */
        type& operator=(type&&) noexcept = default;

        /**
         * Gets the optional type parameter.
         * @return Returns the optional type.
         */
        std::unique_ptr<values::type const> const& parameter() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Type).
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
        std::unique_ptr<values::type const> _parameter;
    };

    /**
     * Stream insertion operator for "type" type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, types::type const& type);

    /**
     * Equality operator for type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(type const& left, type const& right);

    /**
     * Hashes the 'type' type.
     * @param type The 'type' type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(types::type const& type);

}}}  // namespace puppet::runtime::types
