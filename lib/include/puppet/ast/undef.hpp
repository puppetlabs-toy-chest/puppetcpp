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
         * @name position The position of the undef keyword.
         */
        undef(lexer::position position);

        /**
         * The position of the undef keyword.
         */
        lexer::position position;
    };

    /**
     * Stream insertion operator for AST undef.
     * @param os The output stream to write the undef to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, undef const&);

}}  // namespace puppet::ast
