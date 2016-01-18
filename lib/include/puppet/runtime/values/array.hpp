/**
 * @file
 * Declares the array runtime value.
 */
#pragma once

#include "wrapper.hpp"
#include <ostream>
#include <vector>
#include <memory>

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents a runtime array value.
     */
    struct array : std::vector<wrapper<value>>
    {
        // Use the base constructor and assignment semantics
        using vector::vector;
        using vector::operator=;

        /**
         * Joins the array by converting each element to a string.
         * @param os The output stream to write to.
         * @param separator The separator to write between array elements.
         */
        void join(std::ostream& os, std::string const& separator = " ") const;
    };

    /**
     * Stream insertion operator for runtime array.
     * @param os The output stream to write the runtime array to.
     * @param array The runtime array to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, values::array const& array);

    /**
     * Equality operator for array.
     * @param left The left array to compare.
     * @param right The right array to compare.
     * @return Returns true if all elements of the array are equal or false if not.
     */
    bool operator==(array const& left, array const& right);

    /**
     * Inequality operator for array.
     * @param left The left array to compare.
     * @param right The right array to compare.
     * @return Returns true if any elements of the arrays are not equal or false if all elements are equal.
     */
    bool operator!=(array const& left, array const& right);

    /**
     * Hashes the array value.
     * @param array The array value to hash.
     * @return Returns the hash value for the value.
     */
    size_t hash_value(values::array const& array);

}}}  // namespace puppet::runtime::values
