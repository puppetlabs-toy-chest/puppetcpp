/**
 * @file
 * Declares a Unicode string utility type.
 */
#pragma once

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/optional.hpp>
#include <unicode/utext.h>
#include <unicode/ubrk.h>
#include <string>
#include <exception>
#include <limits>

namespace puppet { namespace unicode {

    // Forward declaration of string
    struct string;

    /**
     * Exception for unicode errors.
     */
    struct unicode_exception : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    /**
     * An iterator type for iterating over graphemes in a UTF-8 encoded string.
     * This iterator outputs an iterator range of two character pointers in the original string's range.
     * The range represents the UTF-8 code units that makes up a single Unicode grapheme.
     */
    struct string_iterator :
        boost::iterator_facade<
            string_iterator,
            typename boost::iterator_range<char const*> const,
            boost::forward_traversal_tag
        >
    {
        /**
         * Constructs an empty iterator.
         * An empty iterator is semantically at the "end".
         */
        string_iterator() = default;

        /**
         * Constructs an iterator for the given UTF-8 encoded string.
         * @param string The string being iterated.
         * @param iterate_units Iterates the code units of the UTF-8 data instead of graphemes.
         * @param reversed True to iterate in reverse or false to iterate in a forward fashion.
         */
        explicit string_iterator(std::string const& string, bool iterate_units = false, bool reversed = false);

        /**
         * Constructs an iterator for the given UTF-8 encoded string data.
         * @param data The UTF-8 string data being iterated.
         * @param length The length of the data, in code units (i.e. bytes).
         * @param iterate_units Iterates the code units of the UTF-8 data instead of graphemes.
         * @param reversed True to iterate in reverse or false to iterate in a forward fashion.
         */
        string_iterator(char const* data, size_t length, bool iterate_units = false, bool reversed = false);

        /**
         * Copy constructor for string iterator.
         * @param other The other string iterator to copy.
         */
        string_iterator(string_iterator const& other);

        /**
         * Move constructor for string iterator.
         * @param other The other string iterator to move.
         */
        string_iterator(string_iterator&& other) noexcept;

        /**
         * Copy assignment operator for string iterator.
         * @param other The other string iterator to copy.
         * @return Returns this string iterator.
         */
        string_iterator& operator=(string_iterator const& other);

        /**
         * Move assignment operator for string iterator.
         * @param other The other string iterator to move.
         * @return Returns this string iterator.
         */
        string_iterator& operator=(string_iterator&& other) noexcept;

        /**
         * Destructor for string iterator.
         */
        ~string_iterator();

     private:
        friend class boost::iterator_core_access;

        void increment();
        bool equal(string_iterator const& other) const;
        reference dereference() const;
        void close(bool release = true);

        char const* _data = nullptr;
        size_t _length = 0;
        UText _text = UTEXT_INITIALIZER;
        UBreakIterator* _iterator = nullptr;
        boost::iterator_range<char const*> _value;
        bool _iterate_units = false;
        bool _reversed = false;
    };

    /**
     * An iterator type for splitting a UTF-8 encoded string by Unicode graphemes.
     * This iterator outputs an iterator range of two character pointers in the original string's range.
     * The range represents the split part.  An empty range indicates sequential delimiters.
     */
    struct split_iterator :
        boost::iterator_facade<
            split_iterator,
            typename boost::iterator_range<char const*> const,
            boost::forward_traversal_tag
        >
    {
        /**
         * Constructs an empty iterator.
         * An empty iterator is semantically at the "end".
         */
        split_iterator() = default;

        /**
         * Constructs a split iterator.
         * @param string The string being split.
         * @param delimiter The delimiter string.
         * @param delimiter_length The length of the delimiter, in code units.
         * @param ignore_case True to ignore case differences or false to respect case differences in the search.
         */
        split_iterator(unicode::string const& string, char const* delimiter, size_t delimiter_length, bool ignore_case = false);

     private:
        friend class boost::iterator_core_access;

        void increment();
        bool equal(split_iterator const& other) const;
        reference dereference() const;
        void close();

        string const* _string = nullptr;
        char const* _delimiter = nullptr;
        size_t _delimiter_length = 0;
        char const* _start = nullptr;
        boost::iterator_range<char const*> _value;
        bool _ignore_case = false;
    };

    /**
     * A utility type to handle UTF-8 encoded strings.
     * This type can be used to properly handle unnormalized Unicode graphemes.
     * The string data used by this utility type is stored externally (i.e this
     * type does not store an internal copy of the string data).
     */
    struct string
    {
        /**
         * Represents the const iterator for the string type.
         */
        using const_iterator = unicode::string_iterator;

        /**
         * Represents the const reverse iterator for the string type.
         */
        using const_reverse_iterator = unicode::string_iterator;

        /**
         * Represents the value type (segments representing Unicode graphemes)
         */
        using value_type = unicode::string_iterator::value_type;

        /**
         * Represents the greatest possible position in the string.
         */
        static const size_t npos;

        /**
         * Constructs a Unicode string from a UTF-8 encoded std::string.
         * @param data The string containing the UTF-8 encoded data.
         */
        explicit string(std::string const& data);

        /**
         * Constructs a Unicode string from a UTF-8 encoded null terminated C-string.
         * @param data THe string containing the UTF-8 encoded data.
         */
        explicit string(char const* data);

        /**
         * Gets an iterator to the beginning.
         * @return Returns an iterator to the beginning.
         */
        const_iterator begin() const;

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
        const_reverse_iterator rbegin() const;

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
         * Gets the number of graphemes in the string.
         * @return Returns the number of graphemes in the string.
         */
        size_t graphemes() const noexcept;

        /**
         * Gets the number of code units (i.e. bytes) in the string.
         * @return Returns the size of the string in code units.
         */
        size_t units() const noexcept;

        /**
         * Determines if the string is empty.
         * @return Returns true if the string is empty or false if not.
         */
        bool empty() const noexcept;

        /**
         * Determines if the string is invariant (i.e. contains only ASCII characters).
         * @return Returns true if the string is invariant or false if not.
         */
        bool invariant() const noexcept;

        /**
         * Gets the pointer to the start of the string's UTF-8 data.
         * @return Returns a pointer to the start of the string's UTF-8 data.
         */
        char const* data() const noexcept;

        /**
         * Gets a substring of the original string.
         * Note: unlike std::string, this is an O(N) operation in terms of locating the substring's range.
         * @param start The starting grapheme position.
         * @param length The length of the substring, in graphemes.
         * @return Returns the requested substring.
         */
        std::string substr(size_t start = 0, size_t length = npos) const;

        /**
         * Compares this string to the given Unicode string.
         * @param other The other string to compare.
         * @param ignore_case True to ignore case differences or false to respect case differences in the comparison.
         * @return Returns <0 if the given string is "less than" this string, 0 if they are equal, or >0 if the given string is "greater than".
         */
        int compare(string const& other, bool ignore_case = false) const;

        /**
         * Compares this string to the given UTF-8 encoded std::string.
         * @param other The other string to compare.
         * @param ignore_case True to ignore case differences or false to respect case differences in the comparison.
         * @return Returns <0 if the given string is "less than" this string, 0 if they are equal, or >0 if the given string is "greater than".
         */
        int compare(std::string const& other, bool ignore_case = false) const;

        /**
         * Compares this string to the given UTF-8 encoded C-string.
         * @param other The other string to compare.
         * @param ignore_case True to ignore case differences or false to respect case differences in the comparison.
         * @return Returns <0 if the given string is "less than" this string, 0 if they are equal, or >0 if the given string is "greater than".
         */
        int compare(char const* other, bool ignore_case = false) const;

        /**
         * Determines if this string starts with the other string.
         * @param other The other string.
         * @return Returns true if this string starts with the other string or false if not.
         */
        bool starts_with(unicode::string const& other) const;

        /**
         * Determines if this string starts with the other string.
         * @param other The other string.
         * @return Returns true if this string starts with the other string or false if not.
         */
        bool starts_with(std::string const& other) const;

        /**
         * Determines if this string starts with the other string.
         * @param other The other string.
         * @return Returns true if this string starts with the other string or false if not.
         */
        bool starts_with(char const* other) const;

        /**
         * Converts the string to lowercase.
         * @return Returns the lowercased string.
         */
        std::string lowercase() const;

        /**
         * Converts the string to uppercase.
         * @return Returns the uppercased string.
         */
        std::string uppercase() const;

        /**
         * Capitalizes the first grapheme of the string only.
         * Note: this is like Unicode title casing, but this is limited to only the first word.
         * @return Returns the capitalized string.
         */
        std::string capitalize() const;

        /**
         * Capitalizes the first grapheme following '::' segments.
         * This is typically used for Puppet type names.
         * @return Returns the segment capitalized string.
         */
        std::string capitalize_segments() const;

        /**
         * Trims whitespace from the left (start) of the string.
         * @return Returns the trimmed string.
         */
        std::string trim_left() const;

        /**
         * Trims whitespace from the right (end) of the string.
         * @return Returns the trimmed string.
         */
        std::string trim_right() const;

        /**
         * Trims whitespace from both sides (start and end) of the string.
         * @return Returns the trimmed string.
         */
        std::string trim() const;

        /**
         * Determines if any of the given graphemes are contained in this string.
         * @param graphemes The string containing the graphemes to check.
         * @return Returns true if any of the graphemes are contained in the string or false if none of the graphemes are contained.
         */
        bool contains_any(string const& graphemes) const;

        /**
         * Determines if any of the given graphemes are contained in this string.
         * @param graphemes The UTF-8 encoded string containing the graphemes to check.
         * @return Returns true if any of the graphemes are contained in the string or false if none of the graphemes are contained.
         */
        bool contains_any(std::string const& graphemes) const;

        /**
         * Determines if any of the given graphemes are contained in this string.
         * @param graphemes The UTF-8 encoded string containing the graphemes to check.
         * @return Returns true if any of the graphemes are contained in the string or false if none of the graphemes are contained.
         */
        bool contains_any(char const* graphemes) const;

        /**
         * Finds the range of the given substring.
         * @param substring The substring to find.
         * @param ignore_case True to ignore case differences or false to respect case differences in the search.
         * @return Returns the range of the graphemes matching the given substring or an empty range if not found.
         */
        value_type find(string const& substring, bool ignore_case = false) const;

        /**
         * Finds the first code unit position (i.e. byte offset) of the given substring.
         * @param substring The substring to find.
         * @param ignore_case True to ignore case differences or false to respect case differences in the search.
         * @return Returns the range of the graphemes matching the given substring or an empty range if not found.
         */
        value_type find(std::string const& substring, bool ignore_case = false) const;

        /**
         * Finds the first code unit position (i.e. byte offset) of the given substring.
         * @param substring The substring to find.
         * @param ignore_case True to ignore case differences or false to respect case differences in the search.
         * @return Returns the range of the graphemes matching the given substring or an empty range if not found.
         */
        value_type find(char const* substring, bool ignore_case = false) const;

        /**
         * Gets a beginning split iterator based on the given delimiter.
         * @param delimiter The delimiter to split the string with.
         * @param ignore_case True to ignore case differences or false to respect case differences in the search.
         * @return Returns a beginning split iterator.
         */
        split_iterator split_begin(string const& delimiter, bool ignore_case = false);

        /**
         * Gets a beginning split iterator based on the given delimiter.
         * @param delimiter The delimiter to split the string with.
         * @param ignore_case True to ignore case differences or false to respect case differences in the search.
         * @return Returns a beginning split iterator.
         */
        split_iterator split_begin(std::string const& delimiter, bool ignore_case = false);

        /**
         * Gets a beginning split iterator based on the given delimiter.
         * @param delimiter The delimiter to split the string with.
         * @param ignore_case True to ignore case differences or false to respect case differences in the search.
         * @return Returns a beginning split iterator.
         */
        split_iterator split_begin(char const* delimiter, bool ignore_case = false);

        /**
         * Gets an ending split iterator.
         * @return Returns an ending split iterator.
         */
        split_iterator split_end() const;

        /**
         * Gets the "end of string" pointer (exclusive).
         * @return Returns the end of string pointer.
         */
        char const* eos() const noexcept;

        /**
         * Appends a codepoint in UTF-8 to the given string.
         * @param codepoint The codepoint to append.
         * @param string The string to append the UTF-8 code units to.
         * @return Returns true if the codepoint was valid or false if not.
         */
        static bool append_utf8(char32_t codepoint, std::string& string);

     private:
        friend struct split_iterator;
        bool starts_with(char const* data, size_t length) const;
        std::string trim(bool left, bool right) const;
        bool contains_any(char const* data, size_t length) const;
        value_type find(char const* start, char const* data, size_t length, bool ignore_case) const;
        void count_graphemes();

        char const* _data;
        size_t _graphemes;
        size_t _units;
    };

    /**
     * Equality operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the two strings are equal or false if not.
     */
    bool operator==(string const& left, string const& right);

    /**
     * Equality operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the two strings are equal or false if not.
     */
    bool operator==(string const& left, std::string const& right);

    /**
     * Equality operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the two strings are equal or false if not.
     */
    bool operator==(string const& left, char const* right);

    /**
     * Equality operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the two strings are equal or false if not.
     */
    bool operator==(std::string const& left, string const& right);

    /**
     * Equality operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the two strings are equal or false if not.
     */
    bool operator==(char const* left, string const& right);

    /**
     * Inequality operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the two strings are not equal or false if they are equal.
     */
    bool operator!=(string const& left, string const& right);

    /**
     * Inequality operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the two strings are not equal or false if they are equal.
     */
    bool operator!=(string const& left, std::string const& right);

    /**
     * Inequality operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the two strings are not equal or false if they are equal.
     */
    bool operator!=(string const& left, char const* right);

    /**
     * Inequality operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the two strings are not equal or false if they are equal.
     */
    bool operator!=(std::string const& left, string const& right);

    /**
     * Inequality operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the two strings are not equal or false if they are equal.
     */
    bool operator!=(char const* left, string const& right);

    /**
     * Less than operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is less than the right or false if not.
     */
    bool operator<(string const& left, string const& right);

    /**
     * Less than operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is less than the right or false if not.
     */
    bool operator<(string const& left, std::string const& right);

    /**
     * Less than operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is less than the right or false if not.
     */
    bool operator<(string const& left, char const* right);

    /**
     * Less than operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is less than the right or false if not.
     */
    bool operator<(std::string const& left, string const& right);

    /**
     * Less than operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is less than the right or false if not.
     */
    bool operator<(char const* left, string const& right);

    /**
     * Less than or equal to operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is less than or equal to the right or false if not.
     */
    bool operator<=(string const& left, string const& right);

    /**
     * Less than or equal to operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is less than or equal to the right or false if not.
     */
    bool operator<=(string const& left, std::string const& right);

    /**
     * Less than or equal to operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is less than or equal to the right or false if not.
     */
    bool operator<=(string const& left, char const* right);

    /**
     * Less than or equal to operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is less than or equal to the right or false if not.
     */
    bool operator<=(std::string const& left, string const& right);

    /**
     * Less than or equal to operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is less than or equal to the right or false if not.
     */
    bool operator<=(char const* left, string const& right);

    /**
     * Greater than operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is greater than the right or false if not.
     */
    bool operator>(string const& left, string const& right);

    /**
     * Greater than operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is greater than the right or false if not.
     */
    bool operator>(string const& left, std::string const& right);

    /**
     * Greater than operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is greater than the right or false if not.
     */
    bool operator>(string const& left, char const* right);

    /**
     * Greater than operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is greater than the right or false if not.
     */
    bool operator>(std::string const& left, string const& right);

    /**
     * Greater than operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is greater than the right or false if not.
     */
    bool operator>(char const* left, string const& right);

    /**
     * Greater than or equal to operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is greater than or equal to the right or false if not.
     */
    bool operator>=(string const& left, string const& right);

    /**
     * Greater than or equal to operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is greater than or equal to the right or false if not.
     */
    bool operator>=(string const& left, std::string const& right);

    /**
     * Greater than or equal to operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is greater than or equal to the right or false if not.
     */
    bool operator>=(string const& left, char const* right);

    /**
     * Greater than or equal to operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is greater than or equal to the right or false if not.
     */
    bool operator>=(std::string const& left, string const& right);

    /**
     * Greater than or equal to operator for string.
     * @param left The left string to compare.
     * @param right The right string to compare.
     * @return Returns true if the left is greater than or equal to the right or false if not.
     */
    bool operator>=(char const* left, string const& right);

    /**
     * Stream insertion operator for string.
     * @param os The output stream to write the runtime string to.
     * @param string The runtime string to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, unicode::string const& string);

    /**
     * Hashes the string value.
     * @param string The string value to hash.
     * @return Returns the hash value for the value.
     */
    size_t hash_value(unicode::string const& string);

}}  // namespace puppet::unicode
