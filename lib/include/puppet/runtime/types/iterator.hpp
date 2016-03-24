/**
 * @file
 * Declares the Iterator type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Iterator type.
     */
    struct iterator
    {
        /**
         * Constructs an Iterator type.
         * @param type The iterator value type.
         */
        explicit iterator(std::unique_ptr<values::type> type = nullptr);

        /**
         * Copy constructor for iterator type.
         * @param other The other iterator type to copy from.
         */
        iterator(iterator const& other);

        /**
         * Move constructor for iterator type.
         * Uses the default implementation.
         */
        iterator(iterator&&) noexcept = default;

        /**
         * Copy assignment operator for iterator type.
         * @param other The other iterator type to copy assign from.
         * @return Returns this iterator type.
         */
        iterator& operator=(iterator const& other);

        /**
         * Move assignment operator for iterator type.
         * Uses the default implementation.
         * @return Returns this iterator type.
         */
        iterator& operator=(iterator&&) noexcept = default;

        /**
         * Gets the iterator value type.
         * @return Returns the iterator value type
         */
        std::unique_ptr<values::type> const& type() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Iterator).
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
     * Stream insertion operator for iterator type.
     * @param os The output stream to write the type to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, iterator const&);

    /**
     * Equality operator for iterator.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two iterator types are equal or false if not.
     */
    bool operator==(iterator const& left, iterator const& right);

    /**
     * Inequality operator for iterator.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two iterator types are not equal or false if equal.
     */
    bool operator!=(iterator const& left, iterator const& right);

    /**
     * Hashes the iterator type.
     * @param type The iterator type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(iterator const& type);

}}}  // namespace puppet::runtime::types
