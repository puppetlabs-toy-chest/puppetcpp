/**
 * @file
 * Declares the struct type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <vector>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of recursion_guard
    struct recursion_guard;

    /**
     * Represents the Puppet Struct type.
     */
    struct structure
    {
        /**
         * The type used to define a structure's schema.
         */
        using schema_type = std::vector<std::pair<std::unique_ptr<values::type>, std::unique_ptr<values::type>>>;

        /**
         * Constructs a Struct type.
         * @param schema The struct's schema.
         */
        explicit structure(schema_type schema = schema_type());

        /**
         * Copy constructor for structure type.
         * @param other The other structure type to copy from.
         */
        structure(structure const& other);

        /**
         * Move constructor for structure type.
         * Uses the default implementation.
         */
        structure(structure&&) noexcept = default;

        /**
         * Copy assignment operator for structure type.
         * @param other The other structure type to copy assign from.
         * @return Returns this structure type.
         */
        structure& operator=(structure const& other);

        /**
         * Move assignment operator for structure type.
         * Uses the default implementation.
         * @return Returns this structure type.
         */
        structure& operator=(structure&&) noexcept = default;

        /**
         * Gets the struct's schema.
         * @return Returns the struct's schema.
         */
        schema_type const& schema() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Struct).
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
         * Gets the string representation of a schema key type.
         * @param type The key type to get the key string for.
         * @return Returns the key represented as a string or an empty string if the key is the expected type.
         */
        static std::string const& to_key(values::type const& type);

     private:
        schema_type _schema;
    };

    /**
     * Stream insertion operator for struct type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, structure const& type);

    /**
     * Equality operator for struct.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(structure const& left, structure const& right);

    /**
     * Inequality operator for struct.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(structure const& left, structure const& right);

    /**
     * Hashes the structure type.
     * @param type The structure type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(structure const& type);


}}}  // namespace puppet::runtime::types
