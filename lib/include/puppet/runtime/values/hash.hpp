/**
 * @file
 * Declares the hash runtime value.
 */
#pragma once

#include "wrapper.hpp"
#include <ostream>
#include <unordered_map>
#include <functional>
#include <boost/functional/hash.hpp>

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents a runtime hash value.
     */
    struct hash : std::unordered_map<wrapper<value>, wrapper<value>, boost::hash<wrapper<value>>>
    {
        // Use the base constructor and assignment semantics
        using unordered_map::unordered_map;
        using unordered_map::operator=;
    };

    /**
     * Stream insertion operator for runtime hash.
     * @param os The output stream to write the runtime hash to.
     * @param hash The runtime hash to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, values::hash const& hash);

    /**
     * Equality operator for hash.
     * @param left The left hash to compare.
     * @param right The right hash to compare.
     * @return Returns true if all keys and values of the hash are equal or false if not.
     */
    bool operator==(hash const& left, hash const& right);

    /**
     * Inequality operator for hash.
     * @param left The left hash to compare.
     * @param right The right hash to compare.
     * @return Returns true if any key and value differs or false if they are equal.
     */
    bool operator!=(hash const& left, hash const& right);

    /**
     * Hashes the hash value.
     * @param hash The hash value to hash.
     * @return Returns the hash value for the value.
     */
    size_t hash_value(values::hash const& hash);

}}}  // namespace puppet::runtime::values
