/**
 * @file
 * Declares the AST defined type expression.
 */
#pragma once

#include "expression.hpp"
#include "parameter.hpp"
#include <vector>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents an AST defined type expression.
     */
    struct defined_type_expression
    {
        /**
         * Default constructor for defined_type_expression.
         */
        defined_type_expression();

        /**
         * Constructs a defined type expression with the given position, name, parameters, and body.
         * @param position The position of the expression.
         * @param name The name of the type.
         * @param parameters The optional type parameters.
         * @param body The defined type's body expressions.
         */
        defined_type_expression(lexer::position position, ast::name name, boost::optional<std::vector<parameter>> parameters, boost::optional<std::vector<expression>> body);

        /**
         * The position of the defined type.
         */
        lexer::position position;

        /**
         * The name of the defined type.
         */
        ast::name name;

        /**
         * The parameters of the defined type.
         */
        boost::optional<std::vector<parameter>> parameters;

        /**
         * The body of the defined type.
         */
        boost::optional<std::vector<expression>> body;
    };

    /**
     * Stream insertion operator for AST defined type expression.
     * @param os The output stream to write the expression to.
     * @param expr The expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, defined_type_expression const& expr);

}}  // namespace puppet::ast
