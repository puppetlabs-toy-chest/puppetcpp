/**
 * @file
 * Declares the hash runtime value.
 */
#pragma once

#include "wrapper.hpp"
#include "../../utility/indirect_collection.hpp"
#include <boost/functional/hash.hpp>
#include <ostream>
#include <functional>
#include <list>
#include <unordered_map>

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents a runtime hash value.
     * This models a Ruby hash in that it maintains insertion order but provides an O(1) lookup.
     */
    struct hash
    {
        /**
         * Represents a hash pair.
         */
        struct pair
        {
            /**
             * Constructs a hash pair.
             * @param key The element key.
             * @param value The element value.
             */
            pair(values::value key, values::value value);

            /**
             * Gets the key of the hash pair.
             * @return Returns the key of the hash pair.
             */
            values::value const& key() const;

            /**
             * Gets the value of the hash pair.
             * @return Returns the value of the hash pair.
             */
            values::value& value();

            /**
             * Gets the value of the hash pair.
             * @return Returns the value of the hash pair.
             */
            values::value const& value() const;

         private:
            wrapper<values::value> _key;
            wrapper<values::value> _value;
        };

        /**
         * The underlying sequence type.
         * This is a list because the hash index keeps list references and they cannot be invalidated unless erased.
         * This also provides for a faster erase implementation than a deque could provide.
         */
        using sequence_type = std::list<pair>;

        /**
         * The iterator type for hash.
         */
        using iterator = typename sequence_type::iterator;

        /**
         * The const iterator type for hash.
         */
        using const_iterator = typename sequence_type::const_iterator;

        /**
         * The reverse iterator type for hash.
         */
        using reverse_iterator = typename sequence_type::reverse_iterator;

        /**
         * The const reverse iterator type for hash.
         */
        using const_reverse_iterator = typename sequence_type::const_reverse_iterator;

        /**
         * Default constructor for hash.
         */
        hash() = default;

        /**
         * Copy constructor for hash.
         * @param other The other hash to copy.
         */
        hash(hash const& other);

        /**
         * Move constructor for hash.
         */
        hash(hash&&) noexcept = default;

        /**
         * Copy assignment operator for hash.
         * @param other The other hash to copy.
         * @return Returns this hash.
         */
        hash& operator=(hash const& other);

        /**
         * Move assignment operator for hash.
         * @return Returns this hash.
         */
        hash& operator=(hash&&) noexcept = default;

        /**
         * Gets an iterator to the beginning.
         * @return Returns an iterator to the beginning.
         */
        iterator begin();

        /**
         * Gets an iterator to the beginning.
         * @return Returns an iterator to the beginning.
         */
        const_iterator begin() const;

        /**
         * Gets an iterator to the end.
         * @return Returns an iterator to the end.
         */
        iterator end();

        /**
         * Gets an iterator to the end.
         * @return Returns an iterator to the end.
         */
        const_iterator end() const;

        /**
         * Gets a const iterator to the beginning.
         * @return Returns a const iterator to the beginning.
         */
        const_iterator cbegin() const;

        /**
         * Gets a const iterator to the end.
         * @return Returns a const iterator to the end.
         */
        const_iterator cend() const;

        /**
         * Gets a reverse iterator to the beginning.
         * @return Returns a reverse iterator to the beginning.
         */
        reverse_iterator rbegin();

        /**
         * Gets a reverse iterator to the beginning.
         * @return Returns a reverse iterator to the beginning.
         */
        const_reverse_iterator rbegin() const;

        /**
         * Gets a reverse iterator to the end.
         * @return Returns a reverse iterator to the end.
         */
        reverse_iterator rend();

        /**
         * Gets a reverse iterator to the end.
         * @return Returns a reverse iterator to the end.
         */
        const_reverse_iterator rend() const;

        /**
         * Gets a const reverse iterator to the beginning.
         * @return Returns a const reverse iterator to the beginning.
         */
        const_reverse_iterator crbegin() const;

        /**
         * Gets a const reverse iterator to the end.
         * @return Returns a const reverse iterator to the end.
         */
        const_reverse_iterator crend() const;

        /**
         * Gets the size of the hash (number of elements).
         * @return Returns the size of the hash.
         */
        size_t size() const;

        /**
         * Determines if the hash is empty.
         * @return Returns true if the hash is empty (no elements) or false if it contains at least one element.
         */
        bool empty() const;

        /**
         * Sets an element in the hash.
         * Existing keys will have the value updated to the given value.
         * @param key The key of the element.
         * @param value The value of the element.
         */
        void set(value key, values::value value);

        /**
         * Sets elements by range.
         * @param begin The beginning of the range.
         * @param end The end of the range.
         */
        void set(const_iterator begin, const_iterator end);

        /**
         * Gets a value from the hash.
         * @param key The key of the element to get the value for.
         * @return Returns a pointer to the value if the key is in the hash or nullptr if the key is not in the hash.
         */
        value* get(value const& key);

        /**
         * Gets a value from the hash.
         * @param key The key of the element to get the value for.
         * @return Returns a pointer to the value if the key is in the hash or nullptr if the key is not in the hash.
         */
        value const* get(value const& key) const;

        /**
         * Erases an element from the hash.
         * @param key The key to erase.
         * @return Returns true if an element was erased or false if no element with the given key exists.
         */
        bool erase(value const& key);

     private:
        sequence_type _elements;
        utility::indirect_map<value, iterator> _index;
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
