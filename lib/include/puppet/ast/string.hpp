/**
 * @file
 * Declares the AST string.
 */
#pragma once

#include "../lexer/string_token.hpp"
#include <iostream>
#include <string>

namespace puppet { namespace ast {

    /**
     * Represents an AST string.
     */
    struct string
    {
        /**
         * Default constructor for string.
         */
        string();

        /**
         * Constructs a string from a string token.
         * @tparam Iterator The underlying iterator type for the token.
         * @param token The string token to construct the string from.
         */
        template <typename Iterator>
        explicit string(puppet::lexer::string_token<Iterator>& token) :
            _position(token.position()),
            _value(token.begin(), token.end()),
            _escapes(token.escapes()),
            _quote(token.quote()),
            _interpolated(token.interpolated()),
            _format(token.format()),
            _margin(token.margin()),
            _remove_break(token.remove_break())
        {
        }

        /**
         * Gets the value of the string.
         * @return Returns the value of the string.
         */
        std::string const& value() const;

        /**
         * Gets the valid escape characters for the string token.
         * @return Returns the valid escape characters for the string token.
         */
        std::string const& escapes() const;

        /**
         * Gets the quote character of the token (null character for heredocs).
         * @return Returns the quote character of the token.
         */
        char quote() const;

        /**
         * Gets whether or not the string should be interpolated.
         * @return Returns whether or not the string should be interpolated.
         */
        bool interpolated() const;

        /**
         * Gets the format of the string token (heredoc tokens only).
         * An empty format means "regular string".
         * @return Returns the format of the string token.
         */
        std::string const& format() const;

        /**
         * Gets the margin of the string token (heredoc tokens only)
         * @return Returns the margin of the string token.
         */
        int margin() const;

        /**
         * Gets whether or not a trailing line break should be removed from the string (heredoc tokens only).
         * @return Returns whether or not to remove a trailing line break.
         */
        bool remove_break() const;

        /**
         * Gets the position of the string.
         * @return Returns the position of the string.
         */
        lexer::position const& position() const;

     private:
        lexer::position _position;
        std::string _value;
        std::string _escapes;
        char _quote;
        bool _interpolated;
        std::string _format;
        int _margin;
        bool _remove_break;
    };

    /**
     * Stream insertion operator for AST string.
     * @param os The output stream to write the string to.
     * @param str The string to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, string const& str);

}}  // namespace puppet::ast
