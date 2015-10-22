/**
 * @file
 * Declares the default runtime value.
 */
#pragma once

#include <ostream>

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents the "default" value.
     * Note: called this because "default" is a keyword in C++.
     */
    struct defaulted
    {
    };

    /**
     * Stream insertion operator for runtime default.
     * @param os The output stream to write the runtime default to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, defaulted const&);

    /**
     * Equality operator for default.
     * @return Always returns true.
     */
    bool operator==(defaulted const&, defaulted const&);

    /**
     * Inequality operator for default.
     * @return Always returns false.
     */
    bool operator!=(array const& left, array const& right);

    /**
     * Hashes the default value.
     * @param variable The default value to hash.
     * @return Returns the hash value for the value.
     */
    size_t hash_value(defaulted const&);

}}}  // namespace puppet::runtime::values
