/**
 * @file
 * Declares the NotUndef type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <memory>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet NotUndef type.
     */
    struct not_undef
    {
        /**
         * Constructs an NotUndef type.
         * @param type The optional type.
         */
        explicit not_undef(std::unique_ptr<values::type> type = nullptr);

        /**
         * Copy constructor for NotUndef type.
         * @param other The other NotUndef type to copy from.
         */
        not_undef(not_undef const& other);

        /**
         * Move constructor for NotUndef type.
         * Uses the default implementation.
         */
        not_undef(not_undef&&) noexcept = default;

        /**
         * Copy assignment operator for NotUndef type.
         * @param other The other NotUndef type to copy assign from.
         * @return Returns this NotUndef type.
         */
        not_undef& operator=(not_undef const& other);

        /**
         * Move assignment operator for NotUndef type.
         * Uses the default implementation.
         * @return Returns this NotUndef type.
         */
        not_undef& operator=(not_undef&&) noexcept = default;

        /**
         * Gets the optional type.
         * @return Returns the optional type.
         */
        std::unique_ptr<values::type> const& type() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. NotUndef).
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

     private:
        std::unique_ptr<values::type> _type;
    };

    /**
     * Stream insertion operator for NotUndef type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, not_undef const& type);

    /**
     * Equality operator for NotUndef type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(not_undef const& left, not_undef const& right);

    /**
     * Inequality operator for NotUndef type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(not_undef const& left, not_undef const& right);

    /**
     * Hashes the NotUndef type.
     * @param type The NotUndef type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(not_undef const& type);

}}}  // namespace puppet::runtime::types
