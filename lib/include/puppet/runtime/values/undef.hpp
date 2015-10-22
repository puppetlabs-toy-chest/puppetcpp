/**
 * @file
 * Declares the undef runtime value.
 */
#pragma once

#include <ostream>

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents the undefined value.
     */
    struct undef
    {
    };

    /**
     * Stream insertion operator for runtime undef.
     * @param os The output stream to write the runtime undef to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, undef const&);

    /**
     * Equality operator for undef.
     * @return Always returns true.
     */
    bool operator==(undef const&, undef const&);

    /**
     * Inequality operator for undef.
     * @return Always returns false.
     */
    bool operator==(undef const&, undef const&);

    /**
     * Hashes the undef value.
     * @param variable The undef value to hash.
     * @return Returns the hash value for the value.
     */
    size_t hash_value(values::undef const&);

}}}  // namespace puppet::runtime::values
