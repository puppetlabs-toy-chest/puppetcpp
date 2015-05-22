/**
 * @file
 * Declares the AST "unless" expression.
 */
#pragma once

#include "../lexer/position.hpp"
#include "expression.hpp"
#include "if_expression.hpp"
#include <boost/optional.hpp>
#include <vector>

namespace puppet { namespace ast {

    /**
     * Represents an AST "unless" expression.
     */
    struct unless_expression
    {
        /**
         * Default constructor for unless_expression.
         */
        unless_expression();

        /**
         * Constructs an AST "unless" expression with the given conditional and optional else expression.
         * @param position The position of the "unless" expression.
         * @param conditional The conditional being tested.
         * @param body The expressions making up the unless' body.
         * @param else_ The optional else expression.
         */
        unless_expression(lexer::position position, expression conditional, boost::optional<std::vector<expression>> body, boost::optional<else_expression> else_);

        /**
         * The position of the unless expression.
         */
        lexer::position position;

        /**
         * The conditional of the unless expression.
         */
        expression conditional;

        /**
         * The body of the unless expression.
         */
        boost::optional<std::vector<expression>> body;

        /**
         * The else of the unless expression.
         */
        boost::optional<else_expression> else_;
    };

    /**
     * Stream insertion operator for AST "unless" expression.
     * @param os The output stream to write the "unless" expression to.
     * @param expr The "unless" expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, unless_expression const& expr);

}}  // namespace puppet::ast
