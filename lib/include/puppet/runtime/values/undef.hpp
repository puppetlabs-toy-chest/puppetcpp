/**
 * @file
 * Declares the undef runtime value.
 */
#pragma once

#include <boost/functional/hash.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents the undefined value.
     */
    struct undef
    {
    };

    /**
     * Equality operator for undef.
     * @return Always returns true.
     */
    bool operator==(undef const&, undef const&);

    /**
     * Stream insertion operator for runtime undef.
     * @param os The output stream to write the runtime undef to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, undef const&);

}}}  // puppet::runtime::values

namespace boost {
    /**
     * Hash specialization for undef.
     */
    template <>
    struct hash<puppet::runtime::values::undef>
    {
        /**
         * Hashes the undef value.
         * Note: all undef values hash the same.
         * @return Returns a constant hash value.
         */
        size_t operator()(puppet::runtime::values::undef const&) const
        {
            return 0;
        }
    };
}
