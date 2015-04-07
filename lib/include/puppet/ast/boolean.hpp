/**
 * @file
 * Declares the AST literal boolean.
 */
#pragma once

#include "../lexer/token_position.hpp"
#include <boost/range.hpp>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents an AST literal boolean.
     */
    struct boolean
    {
        /**
         * Default constructor for boolean.
         */
        boolean();

        /**
         * Constructs a boolean with the given value.
         * @param position The position where the literal boolean appeared.
         * @param value The value of the literal boolean.
         */
        boolean(lexer::token_position position, bool value);

        /**
         * Gets the value of the boolean.
         * @return Returns the value of the boolean.
         */
        bool value() const;

        /**
         * Gets the position of the boolean.
         * @return Returns the position of the boolean.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        bool _value;
    };

    /**
     * Stream insertion operator for AST boolean.
     * @param os The output stream to write the boolean to.
     * @param b The boolean to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, boolean const& b);

}}  // namespace puppet::ast
