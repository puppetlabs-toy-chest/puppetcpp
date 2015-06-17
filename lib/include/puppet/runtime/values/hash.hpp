/**
 * @file
 * Declares the array runtime value.
 */
#pragma once

#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <ostream>
#include <unordered_map>
#include <functional>

namespace puppet { namespace runtime { namespace values {

    /**
     * Responsible for performing hashing in a runtime hash value.
     * This exists because we cannot rely on boost::hash because it will hash a value based of the variant's which().
     * That would prevent runtime variables from reliably hashing as the non-variable values they "point" at.
     * @tparam Value The runtime value type.
     */
    template <class Value>
    struct hasher : std::unary_function<Value, std::size_t>
    {
        /**
         * Called to hash a value.
         * @param value The value to hash.
         * @return Returns the hash of the value.
         */
        std::size_t operator()(Value const& value) const
        {
            // Hash any variables as the values they refer to
            return boost::hash_value(dereference(value));
        }
    };

    /**
     * Responsible for performing equality in a runtime hash value.
     * This exists because we cannot rely on std::equal_to because it will call boost::variant's operator==.
     * That operator will not allow for comparing a variable value with a non-variable value for equality.
     * @tparam Value The runtime value type.
     */
    template<typename Value>
    struct equal_to : std::binary_function<Value, Value, bool>
    {
        /**
         * Compares two values for equality.
         * @param left The left value to compare.
         * @param right The right value to compare.
         * @return Returns true if the two values are equal or false if they are not.
         */
        bool operator()(Value const& left, Value const& right) const
        {
            return equals(left, right);
        }
    };

    /**
     * Represents a runtime hash value.
     * @tparam Value The runtime value type.
     */
    template <typename Value>
    using basic_hash = std::unordered_map<
        Value,
        Value,
        values::hasher<Value>,
        values::equal_to<Value>
    >;

    /**
     * Equality operator for hash.
     * @tparam Value The runtime value type.
     * @param left The left hash to compare.
     * @param right The right hash to compare.
     * @return Returns true if all keys and values of the hash are equal or false if not.
     */
    template <typename Value>
    bool operator==(basic_hash<Value> const& left, basic_hash<Value> const& right)
    {
        if (left.size() != right.size()) {
            return false;
        }
        for (auto const& element : left) {
            // Other hash must have the same key
            auto other = right.find(element.first);
            if (other == right.end()) {
                return false;
            }
            // Values must be equal
            if (!equals(element.second, other->second)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Stream insertion operator for runtime hash.
     * @tparam Value The runtime value type.
     * @param os The output stream to write the runtime hash to.
     * @param hash The runtime hash to write.
     * @return Returns the given output stream.
     */
    template <typename Value>
    std::ostream& operator<<(std::ostream& os, basic_hash<Value> const& hash)
    {
        os << '{';
        bool first = true;
        for (auto const& element : hash) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << element.first << " => " << element.second;
        }
        os << '}';
        return os;
    }

}}}  // puppet::runtime::values

namespace boost
{
    /**
     * Hash specialization for hash type.
     * @tparam Value The runtime value type.
     */
    template <typename Value>
    struct hash<puppet::runtime::values::basic_hash<Value>>
    {
        /**
         * Hashes the given hash type.
         * @param hash The hash type to hash.
         * @return Returns the hash value for the hash type.
         */
        size_t operator()(puppet::runtime::values::basic_hash<Value> const& hash) const
        {
            return hash_range(hash.begin(), hash.end());
        }
    };

}  // namespace boost
