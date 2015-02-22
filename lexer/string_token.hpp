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
     */
    struct string_token
    {
        /**
         * Constructs an empty string token.
         */
        string_token() :
            _interpolated(true),
            _escaped(true)
        {
        }

        /**
         * Constructs a string token.
         * @param position The position in the input source for the token.
         * @param text The text for the string.
         * @param format The format for the string (heredoc only).
         * @param interpolated True if the string should be interpolated or false if not.
         * @param escaped True if the $ character should be escaped or false if not.
         */
        string_token(
            token_position position,
            std::string text,
            std::string format = std::string(),
            bool interpolated = true,
            bool escaped = true) :
                _position(std::move(position)),
                _text(std::move(text)),
                _format(std::move(format)),
                _interpolated(interpolated),
                _escaped(escaped)
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
         * Gets the text of the string token.
         * @return Returns the text of the string token.
         */
        std::string const& text() const
        {
            return _text;
        }

        /**
         * Gets the text of the string token.
         * @return Returns the text of the string token.
         */
        std::string& text()
        {
            return _text;
        }

        /**
         * Gets the format of the string token.
         * An empty format means "regular string".
         * @return Returns the format of the string token.
         */
        std::string const& format() const
        {
            return _format;
        }

        /**
         * Gets the format of the string token.
         * An empty format means "regular string".
         * @return Returns the format of the string token.
         */
        std::string& format()
        {
            return _format;
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
         * Gets whether or not the interpolation character ($) should be escaped.
         * @return Returns whether or not the interpolation character ($) should be escaped.
         */
        bool escaped() const
        {
            return _escaped;
        }

     private:
        token_position _position;
        std::string _text;
        std::string _format;
        bool _interpolated;
        bool _escaped;
    };

    /**
     * Stream insertion operator for string token.
     * @param os The output stream to write the token.
     * @param token The token to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, string_token const& token);

}}  // namespace puppet::lexer

namespace boost { namespace spirit { namespace traits
{
    /**
     * Utility for converting iterator ranges into string tokens.
     */
    template <typename Iterator>
    struct assign_to_attribute_from_iterators<puppet::lexer::string_token, Iterator>
    {
        /**
         * Assigns a string token based on an iterator range.
         * String tokens cannot be assigned from iterator range, only from values.
         * Calling this function will result in a runtime error.
         * @param first The first iterator.
         * @param last The last iterator.
         * @param attr The resulting string token attribute.
         */
        static void call(Iterator const& first, Iterator const& last, puppet::lexer::string_token& attr)
        {
            // This should not get called and only exists for the code to compile
            // Tokens that have string data associated with them should be assigned by value, not iterators
            throw std::runtime_error("attempt to assign string token from iterators.");
        }
    };

}}}  // namespace boost::spirit::traits
