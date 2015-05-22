/**
 * @file
 * Declares the AST postfix expression.
 */
#pragma once

#include "expression.hpp"
#include "selector_expression.hpp"
#include "access_expression.hpp"
#include "method_call_expression.hpp"

namespace puppet { namespace ast {

    /**
     * Represents a postfix subexpression.
     */
    typedef boost::variant<
        selector_expression,
        access_expression,
        method_call_expression
    > postfix_subexpression;

    /**
     * Represents a postfix expression.
     */
    struct postfix_expression
    {
        /**
         * Default constructor for postfix expression.
         */
        postfix_expression();

        /**
         * Constructs a postfix expression.
         * @param primary The primary expression.
         * @param subexpressions The postfix subexpressions to apply.
         */
        postfix_expression(primary_expression primary, std::vector<postfix_subexpression> subexpressions);

        /**
         * The primary expression to the postfix expression.
         */
        primary_expression primary;

        /**
         * The postfix subexpressions.
         */
        std::vector<postfix_subexpression> subexpressions;

        /**
         * Gets the position of the expression.
         */
        lexer::position const& position() const;
    };

    /**
     * Stream insertion operator for AST postfix expression.
     * @param os The output stream to write the postfix expression to.
     * @param expr The selector expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, postfix_expression const& expr);

}}  // puppet::ast
