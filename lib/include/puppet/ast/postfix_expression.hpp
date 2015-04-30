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
         * Gets the primary expression.
         * @return Returns the primary expression.
         */
        primary_expression const& primary() const;

        /**
         * Gets the postfix subexpressions.
         * @return Returns the postfix subexpressions.
         */
        std::vector<postfix_subexpression> const& subexpressions() const;

        /**
         * Gets the position of the postfix expression.
         * @return Returns the position of the postfix expression.
         */
        lexer::token_position const& position() const;

     private:
        primary_expression _primary;
        std::vector<postfix_subexpression> _subexpressions;
    };

    /**
     * Stream insertion operator for AST postfix expression.
     * @param os The output stream to write the postfix expression to.
     * @param expr The selector expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, postfix_expression const& expr);

}}  // puppet::ast
