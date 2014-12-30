/**
 * @file
 * Declares the AST access expression.
 */
#pragma once

#include "../lexer/lexer.hpp"
#include "expression.hpp"
#include <vector>

namespace puppet { namespace ast {

    /**
     * Represents an AST access.
     */
    struct access
    {
        /**
         * Default constructor for access.
         */
        access();

        /**
         * Constructs an access with the given argument expressions.
         * @param position The position of the access expression.
         * @param arguments The argument expressions to access.
         */
        access(lexer::token_position position, std::vector<expression> arguments);

        /**
         * Gets the argument expressions.
         * @return Returns the argument expressions.
         */
        std::vector<expression> const& arguments() const;

        /**
         * Gets the argument expressions.
         * @return Returns the argument expressions.
         */
        std::vector<expression>& arguments();

        /**
         * Gets the position of the access.
         * @return Returns the position of the access.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        std::vector<expression> _arguments;
    };

    /**
     * Stream insertion operator for AST access.
     * @param os The output stream to write the access to.
     * @param expr The access to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::access const& access);

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
         * Constructs an access expression with the given original target and subsequent accesses.
         * @param target The original target the first access is performed on.
         * @param accesses The access chains; the first is accessed on the target, the remainder on each subsequent return value from the previous access.
         */
        access_expression(primary_expression target, std::vector<access> accesses);

        /**
         * Gets the target expression.
         * @return Returns the target expression.
         */
        primary_expression const& target() const;

        /**
         * Gets the target expression.
         * @return Returns the target expression.
         */
        primary_expression& target();

        /**
         * Gets the accesses that make up the expression.
         * @return Returns the accesses in the expression.
         */
        std::vector<access> const& accesses() const;

        /**
         * Gets the accesses that make up the expression.
         * @return Returns the accesses in the expression.
         */
        std::vector<access>& accesses();

        /**
         * Gets the position of the access expression.
         * @return Returns the position of the access expression.
         */
        lexer::token_position const& position() const;

    private:
        primary_expression _target;
        std::vector<access> _accesses;
    };

    /**
     * Stream insertion operator for AST access expression.
     * @param os The output stream to write the access expression to.
     * @param expr The access expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, access_expression const& expr);

}}  // puppet::ast
