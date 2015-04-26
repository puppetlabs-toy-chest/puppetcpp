/**
 * @file
 * Declares the regex runtime value.
 */
#pragma once

#include <boost/functional/hash.hpp>
#include <regex>
#include <string>
#include <ostream>

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents a runtime regex.
     */
    struct regex
    {
        /**
         * Default constructor for regex.
         */
        regex();

        /**
         * Constructs a regex with the given pattern.
         * @param pattern The pattern for the regex.
         */
        explicit regex(std::string pattern);

        /**
         * Gets the pattern for the regex.
         * @return Returns the pattern for the regex.
         */
        std::string const& pattern() const;

        /**
         * Gets the value of the regex.
         * @return Returns the value of the regex.
         */
        std::regex const& value() const;

    private:
        std::string _pattern;
        std::regex _regex;
    };

    /**
     * Equality operator for regex.
     * @param left The left regex to compare.
     * @param right The right regex to compare.
     * @return Returns true if both regexes have the same pattern or false if not.
     */
    bool operator==(regex const& left, regex const& right);

    /**
     * Stream insertion operator for runtime regex.
     * @param os The output stream to write the runtime regex to.
     * @param regx The runtime regex to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, regex const& regx);

}}}  // puppet::runtime::values

namespace boost {
    /**
     * Hash specialization for regex.
     */
    template <>
    struct hash<puppet::runtime::values::regex>
    {
        /**
         * Hashes the regex value.
         * @param regex The regex to hash.
         * @return Returns the hash value for the regex.
         */
        size_t operator()(puppet::runtime::values::regex const& regex) const
        {
            return hash_value(regex.pattern());
        }
    };
}
