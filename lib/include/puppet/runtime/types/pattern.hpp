/**
 * @file
 * Declares the pattern type.
 */
#pragma once

#include "../values/forward.hpp"
#include "../values/regex.hpp"
#include <ostream>
#include <vector>
#include <unordered_map>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of alias
    struct alias;

    /**
     * Represents the Puppet Pattern type.
     */
    struct pattern
    {
        /**
         * Constructs a Pattern type.
         * @param patterns The patterns to match with.
         */
        explicit pattern(std::vector<values::regex> patterns = std::vector<values::regex>());

        /**
         * Gets the patterns to match with.
         * @return Returns the patterns to match with.
         */
        std::vector<values::regex> const& patterns() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Pattern).
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
        std::vector<values::regex> _patterns;
    };

    /**
     * Stream insertion operator for pattern type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, pattern const& type);

    /**
     * Equality operator for pattern.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(pattern const& left, pattern const& right);

    /**
     * Inequality operator for pattern.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(pattern const& left, pattern const& right);

    /**
     * Hashes the pattern type.
     * @param type The pattern type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(pattern const& type);

}}}  // namespace puppet::runtime::types
