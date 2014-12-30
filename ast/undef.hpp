/**
 * @file
 * Declares the AST undef.
 */
#pragma once

#include "../lexer/lexer.hpp"
#include <boost/range.hpp>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents an AST undef.
     */
    struct undef
    {
        /**
         * Default constructor for undef.
         */
        undef();

        /**
         * Constructs the undef with the given position.
         * @param position The position of the undef.
         */
        explicit undef(lexer::token_position position);

        /**
         * Gets the position of the undef.
         * @return Returns the position of the undef.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
    };

    /**
     * Stream insertion operator for AST undef.
     * @param os The output stream to write the undef to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, undef const&);

}}  // namespace puppet::ast
