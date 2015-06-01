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
         * Gets the name of the type.
         * @return Returns the name of the type.
         */
        ast::name const& name() const;

        /**
         * Gets the type parameters.
         * @return Returns the type parameters.
         */
        boost::optional<std::vector<parameter>> const& parameters() const;

        /**
         * Gets the optional body expressions.
         * @return Returns the optional body expressions.
         */
        boost::optional<std::vector<expression>> const& body() const;

        /**
         * Gets the position of the expression.
         * @return Returns the position of the expression.
         */
        lexer::position const& position() const;

    private:
        lexer::position _position;
        ast::name _name;
        boost::optional<std::vector<parameter>> _parameters;
        boost::optional<std::vector<expression>> _body;
    };

    /**
     * Stream insertion operator for AST defined type expression.
     * @param os The output stream to write the expression to.
     * @param expr The expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, defined_type_expression const& expr);

}}  // namespace puppet::ast
