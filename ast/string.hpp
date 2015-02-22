/**
 * @file
 * Declares the AST string.
 */
#pragma once

#include "../lexer/string_token.hpp"
#include <boost/range.hpp>
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
         * Constructs a string from a token.
         * @tparam Iterator The underlying iterator type for the token.
         * @param range The token representing the string.
         */
        template <typename Iterator>
        explicit string(boost::iterator_range<Iterator> const& range) :
            _position(range.begin().position()),
            _value(range.begin(), range.end()),
            _interpolated(false),
            _escaped(false)
        {
        }

        /**
         * Constructs a string from a string token.
         * @param token The string token to construct the string from.
         */
        explicit string(puppet::lexer::string_token& token);

        /**
         * Gets the value of the string.
         * @return Returns the value of the string.
         */
        std::string const& value() const;

        /**
         * Gets the value of the string.
         * @return Returns the value of the string.
         */
        std::string& value();

        /**
         * Gets the format of the string.
         * @return Returns the format of the string.
         */
        std::string const& format() const;

        /**
         * Gets the format of the string.
         * @return Returns the format of the string.
         */
        std::string& format();

        /**
         * Gets whether or not the string should be interpolated.
         * @return Returns true if the string should be interpolated or false if not.
         */
        bool interpolated() const;

        /**
         * Gets whether or not the string should be interpolated.
         * @return Returns true if the string should be interpolated or false if not.
         */
        bool& interpolated();

        /**
         * Gets whether or not the interpolation character ($) should be escaped.
         * @return Returns true if the interpolation character should be escaped or false if not.
         */
        bool escaped() const;

        /**
         * Gets whether or not the interpolation character ($) should be escaped.
         * @return Returns true if the interpolation character should be escaped or false if not.
         */
        bool& escaped();

        /**
         * Gets the position of the string.
         * @return Returns the position of the string.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        std::string _value;
        std::string _format;
        bool _interpolated;
        bool _escaped;
    };

    /**
     * Stream insertion operator for AST string.
     * @param os The output stream to write the string to.
     * @param str The string to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, string const& str);

}}  // namespace puppet::ast
