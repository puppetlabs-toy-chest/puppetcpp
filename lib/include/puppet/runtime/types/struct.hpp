/**
 * @file
 * Declares the struct type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <vector>
#include <unordered_map>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of alias
    struct alias;

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
        static std::string to_key(values::type const& type);

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
