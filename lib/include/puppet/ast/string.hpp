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
            position(token.position()),
            value(token.begin(), token.end()),
            escapes(token.escapes()),
            quote(token.quote()),
            interpolated(token.interpolated()),
            format(token.format()),
            margin(token.margin()),
            remove_break(token.remove_break())
        {
        }

        /**
         * The position of the string.
         */
        lexer::position position;

        /**
         * The value of the string.
         */
        std::string value;

        /**
         * The supported escape characters for the string.
         */
        std::string escapes;

        /**
         * The quote character for the string.
         */
        char quote;

        /**
         * Whether or not the string is interpolated.
         */
        bool interpolated;

        /**
         * The format of the string (empty for regular strings).
         */
        std::string format;

        /**
         * The margin of the string (heredoc only).
         */
        int margin;

        /**
         * Whether or not any trailing line break should be removed (heredoc only).
         */
        bool remove_break;
    };

    /**
     * Stream insertion operator for AST string.
     * @param os The output stream to write the string to.
     * @param str The string to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, string const& str);

}}  // namespace puppet::ast
