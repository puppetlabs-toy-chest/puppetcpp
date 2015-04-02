/**
 * @file
 * Declares the string token type.
 */
#pragma once

#include "token_position.hpp"
#include <boost/spirit/include/lex_lexer.hpp>
#include <string>
#include <ostream>

namespace puppet { namespace lexer {

    /**
     * Represents data about a string token.
     * Used for heredocs and quoted strings.
     * @tparam Iterator The input iterator type.
     */
    template <typename Iterator>
    struct string_token
    {
        /**
         * The input iterator type.
         */
        typedef Iterator iterator_type;

        /**
         * Constructs an empty string token.
         */
        string_token() :
            _interpolated(true),
            _margin(0),
            _remove_break(false)
        {
        }

        /**
         * Constructs a string token.
         * @param position The position in the input source for the token.
         * @param begin The begin iterator for the string text.
         * @param end The end iterator for the string text.
         * @param escapes The valid escape characters for the string token.
         * @param quote The quoting character for the string; null character for heredocs.
         * @param interpolated True if the string should be interpolated or false if not.
         * @param format The format for the string (heredoc only).
         * @param margin The margin for the string (heredoc only).
         * @param remove_break Remove a trailing line break from the string (heredoc only).
         */
        string_token(token_position position, iterator_type begin, iterator_type end, std::string escapes, char quote, bool interpolated = true, std::string format = std::string(), int margin = 0, bool remove_break = false) :
            _position(std::move(position)),
            _begin(std::move(begin)),
            _end(std::move(end)),
            _escapes(std::move(escapes)),
            _quote(quote),
            _interpolated(interpolated),
            _format(std::move(format)),
            _margin(margin),
            _remove_break(remove_break)
        {
        }

        /**
         * Gets the position of the token.
         * @return Returns the position of the token.
         */
        token_position const& position() const
        {
            return _position;
        }

        /**
         * Gets the begin iterator for the string text.
         * @return Returns the begin iterator for the string text.
         */
        iterator_type const& begin() const
        {
            return _begin;
        }

        /**
         * Gets the end iterator for the string text.
         * @return Returns the end iterator for the string text.
         */
        iterator_type const& end() const
        {
            return _end;
        }

        /**
         * Gets the valid escape characters for the string token.
         * @return Returns the valid escape characters for the string token.
         */
        std::string const& escapes() const
        {
            return _escapes;
        }

        /**
         * Gets the quote character of the token (null character for heredocs).
         * @return Returns the quote character of the token.
         */
        char quote() const
        {
            return _quote;
        }

        /**
         * Gets whether or not the string should be interpolated.
         * @return Returns whether or not the string should be interpolated.
         */
        bool interpolated() const
        {
            return _interpolated;
        }

        /**
         * Gets the format of the string token (heredoc tokens only).
         * An empty format means "regular string".
         * @return Returns the format of the string token.
         */
        std::string const& format() const
        {
            return _format;
        }

        /**
         * Gets the margin of the string token (heredoc tokens only)
         * @return Returns the margin of the string token.
         */
        int margin() const
        {
            return _margin;
        }

        /**
         * Gets whether or not a trailing line break should be removed from the string (heredoc tokens only).
         * @return Returns whether or not to remove a trailing line break.
         */
        bool remove_break() const
        {
            return _remove_break;
        }

     private:
        token_position _position;
        iterator_type _begin;
        iterator_type _end;
        std::string _escapes;
        char _quote;
        bool _interpolated;
        std::string _format;
        int _margin;
        bool _remove_break;
    };

    /**
     * Stream insertion operator for string token.
     * @tparam Iterator The input iterator.
     * @param os The output stream to write the token.
     * @param token The token to write.
     * @return Returns the given output stream.
     */
    template <typename Iterator>
    std::ostream& operator<<(std::ostream& os, string_token<Iterator> const& token)
    {
        os << (token.interpolated() ? '"' : '\'') << boost::make_iterator_range(token.begin(), token.end()) << (token.interpolated() ? '"' : '\'');
        return os;
    }

}}  // namespace puppet::lexer

namespace boost { namespace spirit { namespace traits
{
    /**
     * Utility for converting iterator ranges into string tokens.
     * @tparam Iterator The input iterator.
     */
    template <typename Iterator>
    struct assign_to_attribute_from_iterators<puppet::lexer::string_token<Iterator>, Iterator>
    {
        /**
         * Assigns a string token based on an iterator range.
         * String tokens cannot be assigned from iterator range, only from values.
         * Calling this function will result in a runtime error.
         * @param first The first iterator.
         * @param last The last iterator.
         * @param attr The resulting string token attribute.
         */
        static void call(Iterator const& first, Iterator const& last, puppet::lexer::string_token<Iterator>& attr)
        {
            // This should not get called and only exists for the code to compile
            // Tokens that have string data associated with them should be assigned by value, not iterators
            throw std::runtime_error("attempt to assign string token from iterators.");
        }
    };

}}}  // namespace boost::spirit::traits
