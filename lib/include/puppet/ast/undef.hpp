/**
 * @file
 * Declares the AST "undef".
 */
#pragma once

#include "../lexer/position.hpp"
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents an AST "undef".
     */
    struct undef
    {
        /**
         * Default constructor for undef.
         */
        undef() = default;

        /**
         * Constructs the undef with the given position.
         * @param position The position of the undef keyword.
         */
        undef(lexer::position position);

        /**
         * Gets the position of the undef.
         * @return Returns the position of the undef.
         */
        lexer::position const& position() const;

     private:
        lexer::position _position;
    };

    /**
     * Stream insertion operator for AST undef.
     * @param os The output stream to write the undef to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, undef const&);

}}  // namespace puppet::ast
