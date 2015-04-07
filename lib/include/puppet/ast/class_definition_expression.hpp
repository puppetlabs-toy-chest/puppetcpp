/**
 * @file
 * Declares the AST class definition expression.
 */
#pragma once

#include "expression.hpp"
#include "parameter.hpp"
#include <vector>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents an AST class definition expression.
     */
    struct class_definition_expression
    {
        /**
         * Default constructor for class_definition_expression.
         */
        class_definition_expression();

        /**
         * Constructs a class definition expression with the given name, optional parameters, optional parent, and optional body expressions.
         * @param position The position of the expression.
         * @param name The name of the class.
         * @param parameters The optional class parameters.
         * @param parent The optional parent name.
         * @param body The class body expressions.
         */
        class_definition_expression(lexer::token_position position, ast::name name, boost::optional<std::vector<parameter>> parameters, boost::optional<ast::name> parent, boost::optional<std::vector<expression>> body);

        /**
         * Gets the name of the class.
         * @return Returns the name of the class.
         */
        ast::name const& name() const;

        /**
         * Gets the class parameters.
         * @return Returns the class parameters.
         */
        boost::optional<std::vector<parameter>> const& parameters() const;

        /**
         * Gets the optional parent name.
         * @return Returns the optional parent name.
         */
        boost::optional<ast::name> const& parent() const;

        /**
         * Gets the optional body expressions.
         * @return Returns the optional body expressions.
         */
        boost::optional<std::vector<expression>> const& body() const;

        /**
         * Gets the position of the expression.
         * @return Returns the position of the expression.
         */
        lexer::token_position const& position() const;

    private:
        lexer::token_position _position;
        ast::name _name;
        boost::optional<std::vector<parameter>> _parameters;
        boost::optional<ast::name> _parent;
        boost::optional<std::vector<expression>> _body;
    };

    /**
     * Stream insertion operator for AST class definition expression.
     * @param os The output stream to write the expression to.
     * @param expr The expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, class_definition_expression const& expr);

}}  // namespace puppet::ast
