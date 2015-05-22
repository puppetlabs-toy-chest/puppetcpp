/**
 * @file
 * Declares the AST "default".
 */
#pragma once

#include "../lexer/position.hpp"
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents an AST "default".
     */
    struct defaulted
    {
        /**
         * Default constructor for defaulted.
         */
        defaulted() = default;

        /**
         * Constructs the defaulted with the given position.
         * @name position The position of the default keyword.
         */
        defaulted(lexer::position position);

        /**
         * The position of the default keyword.
         */
        lexer::position position;
    };

    /**
     * Stream insertion operator for AST default.
     * @param os The output stream to write the default to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, defaulted const&);

}}  // namespace puppet::ast
