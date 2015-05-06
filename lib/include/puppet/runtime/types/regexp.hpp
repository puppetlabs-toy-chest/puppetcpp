/**
 * @file
 * Declares the regexp type.
 */
#pragma once

#include "../values/regex.hpp"
#include <boost/functional/hash.hpp>
#include <ostream>
#include <string>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Regexp type.
     */
    struct regexp
    {
        /**
         * Constructs a Regexp type.
         * @param pattern The regex pattern for the type.  If empty, all patterns match.
         */
        explicit regexp(std::string pattern = std::string());

        /**
         * Gets the pattern being matched or empty string if all patterns match.
         * @return Returns the match pattern.
         */
        std::string const& pattern() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Regexp).
         */
        static const char* name();

        /**
         * Determines if the given value is an instance of this type.
         * @tparam Value The type of the runtime value.
         * @param value The value to determine if it is an instance of this type. This value will never be a variable.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        template <typename Value>
        bool is_instance(Value const& value) const
        {
            auto ptr = boost::get<values::regex>(&value);
            if (!ptr) {
                return false;
            }
            if (_pattern.empty()) {
                return true;
            }
            return ptr->pattern() == _pattern;
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @tparam Type The type of runtime type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        template <typename Type>
        bool is_specialization(Type const& other) const
        {
            // If this Regexp has a pattern, the other type cannot be a specialization
            if (!_pattern.empty()) {
                return false;
            }
            // Check that the other type has a pattern
            auto rgx = boost::get<regexp>(&other);
            return rgx && !rgx->pattern().empty();
        }

     private:
        std::string _pattern;
    };

    /**
     * Stream insertion operator for regexp type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, regexp const& type);

    /**
     * Equality operator for regexp.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(regexp const& left, regexp const& right);

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Regexp type.
     */
    template <>
    struct hash<puppet::runtime::types::regexp>
    {
        /**
         * Hashes the Regexp type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::regexp const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::regexp::name());
            hash_combine(seed, type.pattern());
            return seed;
        }
    };
}
