/**
 * @file
 * Declares the indirect set.
 */
#pragma once

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/optional.hpp>
#include <oniguruma.h>
#include <vector>
#include <string>
#include <exception>

namespace puppet { namespace utility {

    /**
     * Exception for regular expressions.
     */
    struct regex_exception : std::runtime_error
    {
        /**
         * Constructs a regular expression exception.
         * @param message The exception message.
         * @param code The error code from Onigmo.
         */
        regex_exception(std::string const& message, int code);

        /**
         * Gets the error code from Onigmo.
         * @return Returns the error code from Onigmo.
         */
        int code() const;

     private:
        int _code;
    };

    /**
     * Represents a regular expression implemented with the Onigmo regular expression library.
     */
    struct regex
    {
        /**
         * Represents regular expression regions.
         * For matching regular expressions, index 0 represents the region of the whole match, and index 1 to N represent the regions of capture groups.
         */
        struct regions
        {
            /**
             * Constructs a regular expression regions.
             */
            regions();

            /**
             * Destructs a regular expression regions.
             */
            ~regions();

            /**
             * Copy constructor for regular expression regions.
             * @param other The other regions to copy.
             */
            regions(regions const& other);

            /**
             * Move constructor for regular expression regions.
             * @param other The other regions to move from.
             */
            regions(regions&& other);

            /**
             * Copy assignment operator for regular expression regions.
             * @param other The other regions to copy.
             * @return Returns this object.
             */
            regions& operator=(regions const& other);

            /**
             * Move assignment operator for regular expression regions.
             * @param other The other regions to move.
             * @return Returns this object.
             */
            regions& operator=(regions&& other);

            /**
             * Gets the count of the regions.
             * @return Returns the count of the regions.
             */
            size_t count() const;

            /**
             * Determines if the given region is empty.
             * @param index The index of the region.
             * @return Returns true if the region is empty or false if not.
             */
            bool empty(size_t index) const;

            /**
             * Gets the beginning offset of a region.
             * @param index The index of the region.
             * @return Returns the beginning offset of a region.
             */
            size_t begin(size_t index) const;

            /**
             * Gets the ending offset of a region.
             * @param index The index of the region.
             * @return Returns the ending offset of a region.
             */
            size_t end(size_t index) const;

            /**
             * Gets the substring for the given string and region index.
             * @param str The string to get the substring for.
             * @param index The index of the region.
             * @return Returns the substring for the given region.
             */
            std::string substring(std::string const& str, size_t index) const;

            /**
             * Creates an array of substrings given an input string.
             * @param str The string to get the substrings for.
             * @return Returns an array of substrings given an input string.
             */
            std::vector<std::string> substrings(std::string const& str) const;

         private:
            friend struct regex;
            OnigRegion _data;
        };

        /**
         * Constructs a regex with the given expression.
         * @param expression The expression for the regex.
         */
        explicit regex(std::string const& expression);

        /**
         * Matches the regular expression against a string.
         * The match is performed against the entire string.
         * @param str The string to match against the regular expression.
         * @param regions The regions to populate; if nullptr, no regions are returned.
         * @return Returns true if the string matched the regular expression or false if it did not.
         */
        bool match(std::string const& str, regex::regions* regions = nullptr) const;

        /**
         * Searches a string for the given regular expression.
         * @param str The string to search.
         * @param regions The regions to populate; if nullptr, no regions are returned.
         * @param offset The offset from the start for the search.
         * @return Returns true if the regular expression was found in the string or false if not.
         */
        bool search(std::string const& str, regex::regions* regions = nullptr, size_t offset = 0) const;

     private:
        // The wrapper is used to share the Onigmo regex_t across all copies of this utility::regex
        // This allows for a simple move and copy semantic as we consider the regex_t to be immutable
        struct wrapper
        {
            wrapper();
            ~wrapper();
            regex_t const& get() const;
            regex_t& get();

         private:
            regex_t _regex;
        };
        std::shared_ptr<wrapper> _wrapper;
    };

    /**
     * A regular expression iterator.
     * This iterator can be used to iterate over multiple matches (and their capture groups) in a string.
     */
    struct regex_iterator :
        boost::iterator_facade<
            regex_iterator,
            boost::iterator_range<std::string::const_iterator> const,
            boost::forward_traversal_tag
        >
    {
        /**
         * Constructs an empty iterator.
         * An empty iterator is semantically at the "end".
         */
        regex_iterator();

        /**
         * Constructs an iterator for iterating over matches in the given string.
         * @param regex The regular expression to search for.
         * @param str The string to search for matches.
         */
        regex_iterator(utility::regex const& regex, std::string const& str);

     private:
        friend class boost::iterator_core_access;

        void increment();
        bool equal(regex_iterator const& other) const;
        reference dereference() const;
        void move_to_end();

        utility::regex const* _regex;
        std::string const* _string;
        utility::regex::regions _regions;
        size_t _offset;
        boost::iterator_range<std::string::const_iterator> _value;
    };

    /**
     * A regular expression split iterator.
     * This iterator can be used to split a string based on a regular expression.
     * Note that if the regular expression contains capture groups, the captures will be returned as part of the sequence.
     * This is how Ruby's String#split works, for some inexplicable reason.
     */
    struct regex_split_iterator :
        boost::iterator_facade<
            regex_split_iterator,
            boost::iterator_range<std::string::const_iterator> const,
            boost::forward_traversal_tag
        >
    {
        /**
         * Constructs an empty iterator.
         * An empty iterator is semantically at the "end" of the range.
         */
        regex_split_iterator();

        /**
         * Constructs an iterator for iterating over split parts in a string.
         * @param regex The regular expression to split with.
         * @param str The string to split.
         */
        regex_split_iterator(utility::regex const& regex, std::string const& str);

     private:
        friend class boost::iterator_core_access;

        void increment();
        bool equal(regex_split_iterator const& other) const;
        reference dereference() const;
        void move_to_end();

        utility::regex const* _regex;
        std::string const* _string;
        utility::regex::regions _regions;
        size_t _offset;
        size_t _region;
        boost::iterator_range<std::string::const_iterator> _value;
    };

}}  // puppet::utility
