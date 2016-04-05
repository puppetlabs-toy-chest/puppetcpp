/**
 * @file
 * Declares the optional type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <unordered_map>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of alias
    struct alias;

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
         * Determines if the given value is an instance of this type.
         * @param value The value to determine if it is an instance of this type.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        bool is_instance(values::value const& value) const;

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(values::type const& other) const;

        /**
         * Determines if the type is real (i.e. actual type vs. an alias/variant that never resolves to an actual type).
         * @param map The map to keep track of encountered type aliases.
         * @return Returns true if the type is real or false if it never resolves to an actual type.
         */
        bool is_real(std::unordered_map<values::type const*, bool>& map) const;

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
