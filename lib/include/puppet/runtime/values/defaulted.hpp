/**
 * @file
 * Declares the default runtime value.
 */
#pragma once

#include <boost/functional/hash.hpp>
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
     * Equality operator for default.
     * @return Always returns true.
     */
    bool operator==(defaulted const&, defaulted const&);

    /**
     * Stream insertion operator for runtime default.
     * @param os The output stream to write the runtime default to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, defaulted const&);

}}}  // puppet::runtime::values

namespace boost {
    /**
     * Hash specialization for defaulted.
     */
    template <>
    struct hash<puppet::runtime::values::defaulted>
    {
        /**
         * Hashes the default value.
         * Note: all default values hash the same.
         * @return Returns a constant hash value.
         */
        size_t operator()(puppet::runtime::values::defaulted const&) const
        {
            return 0;
        }
    };
}
