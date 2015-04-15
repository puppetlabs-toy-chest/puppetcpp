/**
 * @file
 * Declares the AST "default".
 */
#pragma once

#include "../lexer/token_position.hpp"
#include <boost/range.hpp>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents an AST "default".
     * Note: default is a reserved word in C++.
     */
    struct defaulted
    {
        /**
         * Default constructor for defaulted.
         */
        defaulted();

        /**
         * Constructs the defaulted with the given position.
         * @param position The position of the defaulted.
         */
        explicit defaulted(lexer::token_position position);

        /**
         * Gets the position of the defaulted.
         * @return Returns the position of the defaulted.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
    };

    /**
     * Stream insertion operator for AST defaulted.
     * @param os The output stream to write the defaulted to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, defaulted const&);

}}  // namespace puppet::ast
