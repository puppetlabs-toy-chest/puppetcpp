/**
 * @file
 * Declares the AST access expression.
 */
#pragma once

#include "../lexer/position.hpp"
#include "expression.hpp"
#include <vector>

namespace puppet { namespace ast {

    /**
     * Represents an AST access expression.
     */
    struct access_expression
    {
        /**
         * Default constructor for access expression.
         */
        access_expression() = default;

        /**
         * Constructs an access expression with the given argument expressions.
         * @param position The position of the access expression.
         * @param arguments The argument expressions to access.
         */
        access_expression(lexer::position position, std::vector<expression> arguments);

        /**
         * The position of the access expression.
         */
        lexer::position position;

        /**
         * The arguments to the access expression.
         */
        std::vector<expression> arguments;
    };

    /**
     * Stream insertion operator for AST access expression.
     * @param os The output stream to write the access expression to.
     * @param expr The access expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, access_expression const& expr);

}}  // puppet::ast
