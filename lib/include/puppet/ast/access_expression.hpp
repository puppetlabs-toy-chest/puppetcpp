/**
 * @file
 * Declares the AST access expression.
 */
#pragma once

#include "../lexer/token_position.hpp"
#include "expression.hpp"
#include <vector>

namespace puppet { namespace ast {

    /**
     * Represents an AST access expression.
     */
    struct access_expression
    {
        /**
         * Default constructor for access_expression.
         */
        access_expression();

        /**
         * Constructs an access expression with the given argument expressions.
         * @param position The position of the access expression.
         * @param arguments The argument expressions to access.
         */
        access_expression(lexer::token_position position, std::vector<expression> arguments);

        /**
         * Gets the argument expressions.
         * @return Returns the argument expressions.
         */
        std::vector<expression> const& arguments() const;

        /**
         * Gets the position of the access expression.
         * @return Returns the position of the access expression.
         */
        lexer::token_position const& position() const;

    private:
        lexer::token_position _position;
        std::vector<expression> _arguments;
    };

    /**
     * Stream insertion operator for AST access expression.
     * @param os The output stream to write the access expression to.
     * @param expr The access expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, access_expression const& expr);

}}  // puppet::ast
