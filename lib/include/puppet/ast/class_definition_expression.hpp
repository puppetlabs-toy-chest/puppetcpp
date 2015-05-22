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
        class_definition_expression(lexer::position position, ast::name name, boost::optional<std::vector<parameter>> parameters, boost::optional<ast::name> parent, boost::optional<std::vector<expression>> body);

        /**
         * The position of the class definition.
         */
        lexer::position position;

        /**
         * The name of the class.
         */
        ast::name name;

        /**
         * The parameters to the class.
         */
        boost::optional<std::vector<parameter>> parameters;

        /**
         * The optional parent class.
         */
        boost::optional<ast::name> parent;

        /**
         * The body of the class.
         */
        boost::optional<std::vector<expression>> body;
    };

    /**
     * Stream insertion operator for AST class definition expression.
     * @param os The output stream to write the expression to.
     * @param expr The expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, class_definition_expression const& expr);

}}  // namespace puppet::ast
