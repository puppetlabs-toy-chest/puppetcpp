/**
 * @file
 * Declares the pattern type.
 */
#pragma once

#include "../values/forward.hpp"
#include "../values/regex.hpp"
#include <ostream>
#include <vector>

namespace puppet { namespace runtime { namespace types {

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
