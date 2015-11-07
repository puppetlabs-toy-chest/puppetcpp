/**
 * @file
 * Declares the hash type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <memory>
#include <limits>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Hash type.
     */
    struct hash
    {
        /**
         * Constructs a Hash type.
         * @param key_type The key type of the hash.  Defaults to the Scalar type.
         * @param element_type The element type of the hash.  Defaults to the Data type.
         * @param from The "from" type parameter.
         * @param to The "to" type parameter.
         */
        explicit hash(
            std::unique_ptr<values::type> key_type = nullptr,
            std::unique_ptr<values::type> element_type = nullptr,
            int64_t from = std::numeric_limits<int64_t>::min(),
            int64_t to = std::numeric_limits<int64_t>::max());

        /**
         * Copy constructor for hash type.
         * @param other The other hash type to copy from.
         */
        hash(hash const& other);

        /**
         * Move constructor for hash type.
         * Uses the default implementation.
         */
        hash(hash&&) noexcept = default;

        /**
         * Copy assignment operator for hash type.
         * @param other The other hash type to copy assign from.
         * @return Returns this hash type.
         */
        hash& operator=(hash const& other);

        /**
         * Move assignment operator for hash type.
         * Uses the default implementation.
         * @return Returns this hash type.
         */
        hash& operator=(hash&&) noexcept = default;

        /**
         * Gets the key type of the hash.
         * @return Returns the element type of the hash.
         */
        values::type const& key_type() const;

        /**
         * Gets the element type of the hash.
         * @return Returns the element type of the hash.
         */
        values::type const& element_type() const;

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
         * @return Returns the name of the type (i.e. Hash).
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
        std::unique_ptr<values::type> _key_type;
        std::unique_ptr<values::type> _element_type;
        int64_t _from;
        int64_t _to;
    };

    /**
     * Stream insertion operator for hash type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, hash const& type);

    /**
     * Equality operator for hash.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(hash const& left, hash const& right);

    /**
     * Inequality operator for hash.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(hash const& left, hash const& right);

    /**
     * Hashes the 'hash' type.
     * @param type The 'hash' type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(hash const& type);

}}}  // puppet::runtime::types
