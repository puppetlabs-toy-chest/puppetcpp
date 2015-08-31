/**
 * @file
 * Declares the pattern type.
 */
#pragma once

#include "../values/regex.hpp"
#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
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
            // Check for string
            auto ptr = boost::get<std::string>(&value);
            if (!ptr) {
                return false;
            }

            // Check for no patterns (accept any string)
            if (_patterns.empty()) {
                return true;
            }

            // Check for a matching pattern
            for (auto const& regex : _patterns) {
                if (regex.pattern().empty() || std::regex_search(*ptr, regex.value())) {
                    return true;
                }
            }
            return false;
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
            // Specializations of Pattern have *fewer* patterns (i.e. are more restrictive)
            auto ptr = boost::get<pattern>(&other);
            if (!ptr) {
                return false;
            }
            return ptr->patterns().size() < _patterns.size();
        }

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

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Pattern type.
     */
    template <>
    struct hash<puppet::runtime::types::pattern>
    {
        /**
         * Hashes the Pattern type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::pattern const& type) const
        {
            static const size_t name_hash = boost::hash_value(puppet::runtime::types::pattern::name());

            size_t seed = 0;
            hash_combine(seed, name_hash);
            hash_range(seed, type.patterns().begin(), type.patterns().end());
            return seed;
        }
    };
}
