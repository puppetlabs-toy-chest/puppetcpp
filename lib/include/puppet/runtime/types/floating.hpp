/**
 * @file
 * Declares the float type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <limits>
#include <unordered_map>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of alias
    struct alias;

    /**
     * Represents the Puppet Float type.
     */
    struct floating
    {
        /**
         * Constructs a float type.
         * @param from The "from" type parameter.
         * @param to The "to" type parameter.
         */
        explicit floating(double from = std::numeric_limits<double>::lowest(), double to = std::numeric_limits<double>::max());

        /**
         * Gets the "from" type parameter.
         * @return Returns the "from" type parameter.
         */
        double from() const;

        /**
         * Gets the "to" type parameter.
         * @return Returns the "to" type parameter.
         */
        double to() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Float).
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
        double _from;
        double _to;
    };

    /**
     * Stream insertion operator for float type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, floating const& type);

    /**
     * Equality operator for float.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(floating const& left, floating const& right);

    /**
     * Inequality operator for float.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
     bool operator!=(floating const& left, floating const& right);

    /**
     * Hashes the float type.
     * @param type The float type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(floating const& type);

}}}  // namespace puppet::runtime::types
