/**
 * @file
 * Declares the string token type.
 */
#pragma once

#include "position.hpp"
#include "../../cast.hpp"
#include <boost/spirit/include/lex_lexer.hpp>
#include <string>
#include <ostream>

namespace puppet { namespace compiler { namespace lexer {

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
        using iterator_type = Iterator;

        /**
         * The string value type.
         */
        using value_type = boost::iterator_range<Iterator>;

        /**
         * Constructs a string token.
         * @param range The token's range.
         * @param value The iterator range for the string's value.
         * @param escapes The valid escape characters for the string token.
         * @param quote The quoting character for the string; null character for heredocs.
         * @param interpolated True if the string should be interpolated or false if not.
         * @param format The format for the string (heredoc only).
         * @param margin The margin for the string (heredoc only).
         * @param remove_break Remove a trailing line break from the string (heredoc only).
         */
        string_token(lexer::range range, value_type value, std::string escapes, char quote, bool interpolated = true, std::string format = std::string(), int margin = 0, bool remove_break = false) :
            _range(rvalue_cast(range)),
            _value(rvalue_cast(value)),
            _escapes(rvalue_cast(escapes)),
            _quote(quote),
            _interpolated(interpolated),
            _format(rvalue_cast(format)),
            _margin(margin),
            _remove_break(remove_break)
        {
        }

        /**
         * Gets the range of the token.
         * @return Returns the range of the token.
         */
        lexer::range const& range() const
        {
            return _range;
        }

        /**
         * Gets the iterator range representing the string value.
         * @return Returns the iterator range representing the string value.
         */
        value_type const& value() const
        {
            return _value;
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
        lexer::range _range;
        value_type _value;
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
        os << (token.interpolated() ? '"' : '\'') << token.value() << (token.interpolated() ? '"' : '\'');
        return os;
    }

}}}  // namespace puppet::compiler::lexer

namespace boost { namespace spirit { namespace traits
{
    /**
     * Utility for converting iterator ranges into string tokens.
     * @tparam Iterator The input iterator.
     */
    template <typename Iterator>
    struct assign_to_attribute_from_iterators<puppet::compiler::lexer::string_token<Iterator>, Iterator>
    {
        /**
         * Assigns a string token based on an iterator range.
         * String tokens cannot be assigned from iterator range, only from values.
         * Calling this function will result in a runtime error.
         * @param first The first iterator.
         * @param last The last iterator.
         * @param attr The resulting string token attribute.
         */
        static void call(Iterator const& first, Iterator const& last, puppet::compiler::lexer::string_token<Iterator>& attr)
        {
            // This should not get called and only exists for the code to compile
            // Tokens that have string data associated with them should be assigned by value, not iterators
            throw std::runtime_error("attempt to assign string token from iterators.");
        }
    };

}}}  // namespace boost::spirit::traits
