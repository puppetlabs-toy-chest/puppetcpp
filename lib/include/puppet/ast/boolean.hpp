/**
 * @file
 * Declares the AST literal boolean.
 */
#pragma once

#include "../lexer/position.hpp"
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents an AST literal boolean.
     */
    struct boolean
    {
        boolean();

        boolean(lexer::position position, bool value);

        /**
         * The position of the boolean.
         */
        lexer::position position;

        /**
         * The value of the boolean.
         */
        bool value;
    };

    /**
     * Stream insertion operator for AST boolean.
     * @param os The output stream to write the boolean to.
     * @param b The boolean to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, boolean const& b);

}}  // namespace puppet::ast
