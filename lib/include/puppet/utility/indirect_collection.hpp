/**
 * @file
 * Declares the indirect set.
 */
#pragma once

#include <unordered_set>
#include <unordered_map>
#include <boost/functional/hash.hpp>

namespace puppet { namespace utility {

    /**
     * Utility type for indirectly hashing values.
     * @tparam T The value type.
     */
    template <typename T>
    struct indirect_hasher
    {
        /**
         * Called to hash a value.
         * @param value The value to hash.
         * @return Returns the hash value.
         */
        size_t operator()(T const* value) const
        {
            boost::hash<T> hasher;
            return hasher(*value);
        }
    };

    /**
     * Utility type for indirectly comparing values.
     * @tparam T The value type.
     */
    template <typename T>
    struct indirect_comparer
    {
        /**
         * Called to compare two values.
         * @param left The left value to compare.
         * @param right The right value to compare.
         * @return Returns true if the two values are equal or false if they are not equal.
         */
        bool operator()(T const* left, T const* right) const
        {
            return *left == *right;
        }
    };

    /**
     * An indirect, unordered map.
     * @tparam KeyType The key type for the map.
     * @tparam ValueType The value type for the map.
     */
    template <typename KeyType, typename ValueType>
    using indirect_map = std::unordered_map<KeyType const*, ValueType, indirect_hasher<KeyType>, indirect_comparer<KeyType>>;

    /**
     * An indirect, unordered set.
     * @tparam ValueType The value type for the set.
     */
    template <typename ValueType>
    using indirect_set = std::unordered_set<ValueType const*, indirect_hasher<ValueType>, indirect_comparer<ValueType>>;

}}  // puppet::utility
