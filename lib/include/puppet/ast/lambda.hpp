/**
 * @file
 * Declares the AST lambda.
 */
#pragma once

#include "../lexer/position.hpp"
#include "parameter.hpp"
#include <boost/optional.hpp>
#include <vector>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents the type of an AST lambda.
     */
    struct lambda
    {
        /**
         * Default constructor for lambda.
         */
        lambda();

        /**
         * Constructs a lambda with the given position, optional parameters, and optional body expressions.
         * @param position The position of the lambda.
         * @param parameters The optional parameters of the lambda.
         * @param body The optional expressions of the lambda's body.
         */
        lambda(lexer::position position, boost::optional<std::vector<parameter>> parameters, boost::optional<std::vector<expression>> body);

        /**
         * The position of the lambda.
         */
        lexer::position position;

        /**
         * The parameters of the lambda.
         */
        boost::optional<std::vector<parameter>> parameters;

        /**
         * The body of the lambda.
         */
        boost::optional<std::vector<expression>> body;
    };

    /**
     * Stream insertion operator for AST lambda.
     * @param os The output stream to write the lambda to.
     * @param lambda The lambda to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::lambda const& lambda);

}}  // puppet::ast
