/**
 * @file
 * Declares the AST number.
 */
#pragma once

#include "../lexer/number_token.hpp"
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents the AST number.
     */
    struct number
    {
        /**
         * The type for the numeric value.
         */
        typedef typename lexer::number_token::value_type value_type;

        /**
         * Default constructor for number.
         */
        number();

        /**
         * Constructs a number from a token.
         * @param token The token representing the number.
         */
        explicit number(lexer::number_token const& token);

        /**
         * Gets the value of the number.
         * @return Returns the value of the number.
         */
        value_type const& value() const;

        /**
         * Gets the position of the number.
         * @return Returns the position of the number.
         */
        lexer::position const& position() const;

     private:
        lexer::position _position;
        value_type _value;
    };

    /**
     * Stream insertion operator for AST number.
     * @param os The output stream to write the number to.
     * @param number The number to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, number const& number);

}}  // namespace puppet::ast
