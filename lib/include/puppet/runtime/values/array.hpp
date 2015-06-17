/**
 * @file
 * Declares the array runtime value.
 */
#pragma once

#include <ostream>
#include <vector>

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents a runtime array value.
     * @tparam Value The runtime value type.
     */
    template <typename Value>
    using basic_array = std::vector<Value>;

    /**
     * Equality operator for array.
     * @tparam Value The runtime value type.
     * @param left The left array to compare.
     * @param right The right array to compare.
     * @return Returns true if all elements of the array are equal or false if not.
     */
    template <typename Value>
    bool operator==(basic_array<Value> const& left, basic_array<Value> const& right)
    {
        if (left.size() != right.size()) {
            return false;
        }
        for (size_t i = 0; i < left.size(); ++i) {
            if (!equals(left[i], right[i])) {
                return false;
            }
        }
        return true;
    }

    /**
     * Stream insertion operator for runtime array.
     * @tparam Value The runtime value type.
     * @param os The output stream to write the runtime array to.
     * @param array The runtime array to write.
     * @return Returns the given output stream.
     */
    template <typename Value>
    std::ostream& operator<<(std::ostream& os, basic_array<Value> const& array)
    {
        os << '[';
        bool first = true;
        for (auto const& element : array) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << element;
        }
        os << ']';
        return os;
    }

}}}  // puppet::runtime::values
