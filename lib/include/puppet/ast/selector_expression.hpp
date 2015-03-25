/**
 * @file
 * Declares the AST selector expression.
 */
#pragma once

#include "expression.hpp"

namespace puppet { namespace ast {

    /**
     * Represents an AST selector case expression.
     */
    struct selector_case_expression
    {
        /**
         * Default constructor for selector case expression.
         */
        selector_case_expression();

        /**
         * Constructs a selector case expression with the given selector and result expressions.
         * @param selector The selector expression.
         * @param result The result expression if the selector matches.
         */
        selector_case_expression(expression selector, expression result);

        /**
         * Constructs a default selector case expression with the given position and result expression.
         * @param position The position of the default case expression.
         * @param result The result expression if no selector matches.
         */
        selector_case_expression(lexer::token_position position, expression result);

        /**
         * Gets the selector expression.
         * @return Returns the selector expression.
         */
        expression const& selector() const;

        /**
         * Gets the selector expression.
         * @return Returns the selector expression.
         */
        expression& selector();

        /**
         * Gets the result expression.
         * @return Returns the result expression.
         */
        expression const& result() const;

        /**
         * Gets the result expression.
         * @return Returns the result expression.
         */
        expression& result();

        /**
         * Determines if the expression is for the default case.
         * @return Returns true if the expression is for the default case or false if it is not.
         */
        bool is_default() const;

        /**
         * Gets the position of the selector case expression.
         * @return Returns the position of the selector case expression.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        expression _selector;
        expression _result;
    };

    /**
     * Stream insertion operator for AST selector case expression.
     * @param os The output stream to write the selector case expression to.
     * @param expr The selector case expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, selector_case_expression const& expr);

    /**
     * Represents an AST selector expression.
     */
    struct selector_expression
    {
        /**
         * Default constructor for selector_expression.
         */
        selector_expression();

        /**
         * Constructs a selector expression with the given value expression and cases.
         * @param value The value being selected upon.
         * @param cases The selector cases.
         */
        selector_expression(primary_expression value, std::vector<selector_case_expression> cases);

        /**
         * Gets the value of the selector expression.
         * @return Returns the value of the selector expression.
         */
        primary_expression const& value() const;

        /**
         * Gets the value of the selector expression.
         * @return Returns the value of the selector expression.
         */
        primary_expression& value();

        /**
         * Gets the selector case expressions.
         * @return Returns the selector case expressions.
         */
        std::vector<selector_case_expression> const& cases() const;

        /**
         * Gets the selector case expressions.
         * @return Returns the selector case expressions.
         */
        std::vector<selector_case_expression>& cases();

        /**
         * Gets the position of the selector expression.
         * @return Returns the position of the selector expression.
         */
        lexer::token_position const& position() const;

     private:
        primary_expression _value;
        std::vector<selector_case_expression> _cases;
    };

    /**
     * Stream insertion operator for AST selector expression.
     * @param os The output stream to write the selector expression to.
     * @param expr The selector expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, selector_expression const& expr);

}}  // puppet::ast
