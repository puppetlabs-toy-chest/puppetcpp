/**
 * @file
 * Declares the AST number.
 */
#pragma once

#include "../lexer/token_position.hpp"
#include <boost/range.hpp>
#include <iostream>
#include <string>

namespace puppet { namespace ast {

    /**
     * Represents the AST number.
     */
    struct number
    {
        /**
         * Default constructor for number.
         */
        number();

        /**
         * Constructs a number from a token.
         * @tparam Iterator The underlying iterator type for the token.
         * @param token The token representing the number.
         */
        template <typename Iterator>
        explicit number(boost::iterator_range<Iterator> const& token) :
            _position(token.begin().position()),
            _value(token.begin(), token.end())
        {
        }

        /**
         * Gets the value of the number.
         * @return Returns the value of the number.
         */
        std::string const& value() const;

        /**
         * Gets the value of the number.
         * @return Returns the value of the number.
         */
        std::string& value();

        /**
         * Gets the position of the number.
         * @return Returns the position of the number.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        std::string _value;
    };

    /**
     * Stream insertion operator for AST number.
     * @param os The output stream to write the number to.
     * @param number The number to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, number const& number);

}}  // namespace puppet::ast
