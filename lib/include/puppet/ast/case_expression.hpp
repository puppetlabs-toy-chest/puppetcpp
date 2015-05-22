/**
 * @file
 * Declares the AST case expression.
 */
#pragma once

#include "expression.hpp"
#include "lambda.hpp"
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
         * Constructs a case proposition with the given lambda option and body expressions.
         * @param option The case proposition lambda.
         * @param body The expressions that make up the body of the proposition.
         */
        case_proposition(lambda option, boost::optional<std::vector<expression>> body);

        /**
         * The position of the case proposition.
         */
        lexer::position position;

        /**
         * The options of the case proposition.
         */
        std::vector<expression> options;

        /**
         * The lambda of the case proposition.
         */
        boost::optional<ast::lambda> lambda;

        /**
         * The body of the case proposition.
         */
        boost::optional<std::vector<expression>> body;
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
        case_expression(lexer::position position, ast::expression expression, std::vector<case_proposition> propositions);

        /**
         * The position of the case expression.
         */
        lexer::position position;

        /**
         * The case expression.
         */
        ast::expression expression;

        /**
         * The case propositions.
         */
        std::vector<case_proposition> propositions;
    };

    /**
     * Stream insertion operator for AST case expression.
     * @param os The output stream to write the case expression to.
     * @param expr The case expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, case_expression const& expr);

}}  // puppet::ast
