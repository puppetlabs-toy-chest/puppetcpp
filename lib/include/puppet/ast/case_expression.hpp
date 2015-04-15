/**
 * @file
 * Declares the AST case expression.
 */
#pragma once

#include "expression.hpp"
#include <boost/optional.hpp>
#include <vector>

namespace puppet { namespace ast {

    /**
     * Represents an AST case proposition.
     */
    struct case_proposition
    {
        /**
         * Default constructor for case_proposition.
         */
        case_proposition();

        /**
         * Constructs a case proposition with the given option and body expressions.
         * @param options The case proposition expressions.
         * @param body The expressions that make up the body of the proposition.
         */
        case_proposition(std::vector<expression> options, boost::optional<std::vector<expression>> body);

        /**
         * Gets the case proposition options.
         * @return Returns the case proposition options.
         */
        std::vector<expression> const& options() const;

        /**
         * Gets the expressions that make up the body of the proposition.
         * @return Returns the expressions that make up the body of the proposition.
         */
        boost::optional<std::vector<expression>> const& body() const;

        /**
         * Gets the position of the case proposition.
         * @return Returns the position of the case proposition.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        std::vector<expression> _options;
        boost::optional<std::vector<expression>> _body;
    };

    /**
     * Stream insertion operator for AST case proposition.
     * @param os The output stream to write the case proposition to.
     * @param proposition The case proposition to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, case_proposition const& proposition);

    /**
     * Represents an AST case expression.
     */
    struct case_expression
    {
        /**
         * Default constructor for case_expression.
         */
        case_expression();

        /**
         * Constructs a case expression with the given expression and propositions.
         * @param position The position of the case expression.
         * @param expression The case expression.
         * @param propositions The case propositions.
         */
        case_expression(lexer::token_position position, ast::expression expression, std::vector<case_proposition> propositions);

        /**
         * Gets the case expression.
         * @return Returns the case expression.
         */
        ast::expression const& expression() const;

        /**
         * Gets the case propositions.
         * @return Returns the case propositions.
         */
        std::vector<case_proposition> const& propositions() const;

        /**
         * Gets the position of the case expression.
         * @return Returns the position of the case expression.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        struct expression _expression;
        std::vector<case_proposition> _propositions;
    };

    /**
     * Stream insertion operator for AST case expression.
     * @param os The output stream to write the case expression to.
     * @param expr The case expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, case_expression const& expr);

}}  // puppet::ast
