/**
 * @file
 * Declares the AST "if" expression.
 */
#pragma once

#include "../lexer/lexer.hpp"
#include "expression.hpp"
#include <boost/optional.hpp>
#include <vector>

namespace puppet { namespace ast {

    /**
     * Represents the "else" part of an "if" expression.
     */
    struct else_expression
    {
        /**
         * Default constructor of else_expression.
         */
        else_expression();

        /**
         * Constructs an "else" expression with the given optional body expressions.
         * @param position The position of the "else" expression.
         * @param body The optional expressions making up the "else" expression's body.
         */
        else_expression(lexer::token_position position, boost::optional<std::vector<expression>> body);

        /**
         * Gets the optional expressions that make up the body.
         * @return Returns the optional expressions that make up the body.
         */
        boost::optional<std::vector<expression>> const& body() const;

        /**
         * Gets the optional expression that make up the body.
         * @return Returns the optional expression that make up the body.
         */
        boost::optional<std::vector<expression>>& body();

        /**
         * Gets the position of the "else" expression.
         * @return Returns the position of the "else" expression.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        boost::optional<std::vector<expression>> _body;
    };

    /**
     * Stream insertion operator for AST "else" expression.
     * @param os The output stream to write the "else" expression to.
     * @param expr The "else" expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, else_expression const& expr);

    /**
     * Represents the "else if" part of an "if" expression.
     */
    struct elsif_expression
    {
        /**
         * Default constructor for elsif_expression.
         */
        elsif_expression();

        /**
         * Constructs an "else if" expression with the given conditional and optional body expressions.
         * @param position The position of the "else if" expression.
         * @param conditional The conditional to test.
         * @param body The optional expressions making up the "else if" expression's body.
         */
        elsif_expression(lexer::token_position position, expression conditional, boost::optional<std::vector<expression>> body);

        /**
         * Gets the conditional of the "else if" expression.
         * @return Returns the conditional of the "else if" expression.
         */
        expression const& conditional() const;

        /**
         * Gets the conditional of the "else if" expression.
         * @return Returns the conditional of the "else if" expression.
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
         * Gets the position of the "else if" expression.
         * @return Returns the position of the "else if" expression.
         */
        lexer::token_position const& position() const;

    private:
        lexer::token_position _position;
        expression _conditional;
        boost::optional<std::vector<expression>> _body;
    };

    /**
     * Stream insertion operator for AST "else if" expression.
     * @param os The output stream to write the "else if" expression to.
     * @param expr The "else if" expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, elsif_expression const& expr);

    /**
     * Represents an AST "if" expression.
     */
    struct if_expression
    {
        /**
         * Default constructor for if_expression.
         */
        if_expression();

        /**
         * Constructs an AST "if" expression with the given conditional, optional else if expressions, and optional else expression.
         * @param position The position of the "if" expression.
         * @param conditional The conditional being tested.
         * @param body The expressions making up the if's body.
         * @param elsifs The optional list of else if expressions.
         * @param else_ The optional else expression.
         */
        if_expression(
            lexer::token_position position,
            expression conditional,
            boost::optional<std::vector<expression>> body,
            boost::optional<std::vector<elsif_expression>> elsifs,
            boost::optional<else_expression> else_);

        /**
         * Gets the conditional of the "if" expression.
         * @return Returns the conditional of the "if" expression.
         */
        expression const& conditional() const;

        /**
         * Gets the conditional of the "if" expression.
         * @return Returns the conditional of the "if" expression.
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
         * Gets the optional list of "else if" expressions.
         * @return Returns the optional list of "else if" expressions.
         */
        boost::optional<std::vector<elsif_expression>> const& elsifs() const;

        /**
         * Gets the optional list of "else if" expressions.
         * @return Returns the optional list of "else if" expressions.
         */
        boost::optional<std::vector<elsif_expression>>& elsifs();

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
         * Gets the position of the "if" expression.
         * @return Returns the position of the "if" expression.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        expression _conditional;
        boost::optional<std::vector<expression>> _body;
        boost::optional<std::vector<elsif_expression>> _elsifs;
        boost::optional<else_expression> _else;
    };

    /**
     * Stream insertion operator for AST "if" expression.
     * @param os The output stream to write the "if" expression to.
     * @param expr The "if" expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, if_expression const& expr);

}}  // puppet::ast
