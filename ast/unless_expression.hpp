/**
 * @file
 * Declares the AST "unless" expression.
 */
#pragma once

#include "../lexer/lexer.hpp"
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
        unless_expression(lexer::token_position position, expression conditional, boost::optional<std::vector<expression>> body, boost::optional<else_expression> else_);

        /**
         * Gets the conditional of the "unless" expression.
         * @return Returns the conditional of the "unless" expression.
         */
        expression const& conditional() const;

        /**
         * Gets the conditional of the "unless" expression.
         * @return Returns the conditional of the "unless" expression.
         */
        expression& conditional();

        /**
         * Gets the optional expressions that make up the body.
         * @return Returns the optional expressions that make up the body.
         */
        boost::optional<std::vector<expression>> const& body() const;

        /**
         * Gets the optional expressions that make up the body.
         * @return Returns the optional expressions that make up the body.
         */
        boost::optional<std::vector<expression>>& body();

        /**
         * Gets the optional "else" expression.
         * @return Returns the optional "else" expression.
         */
        boost::optional<else_expression> const& else_() const;

        /**
         * Gets the optional "else" expression.
         * @return Returns the optional "else" expression.
         */
        boost::optional<else_expression>& else_();

        /**
         * Gets the position of the "unless" expression.
         * @return Returns the position of the "unless" expression.
         */
        lexer::token_position const& position() const;

    private:
        lexer::token_position _position;
        expression _conditional;
        boost::optional<std::vector<expression>> _body;
        boost::optional<else_expression> _else;
    };

    /**
     * Stream insertion operator for AST "unless" expression.
     * @param os The output stream to write the "unless" expression to.
     * @param expr The "unless" expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, unless_expression const& expr);

}}  // namespace puppet::ast
