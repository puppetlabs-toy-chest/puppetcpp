/**
 * @file
 * Declares the AST method call expression.
 */
#pragma once

#include "../lexer/position.hpp"
#include "expression.hpp"
#include "name.hpp"
#include "lambda.hpp"
#include <boost/optional.hpp>
#include <vector>

namespace puppet { namespace ast {

    /**
     * Represents an AST method call expression.
     */
    struct method_call_expression
    {
        /**
         * Default constructor for method_call_expression.
         */
        method_call_expression();

        /**
         * Constructs a method call expression with the given method name, optional arguments, and optional lambda.
         * @param method The name of the method being called.
         * @param arguments The optional arguments to the method.
         * @param lambda The optional lambda to the method.
         */
        method_call_expression(name method, boost::optional<std::vector<expression>> arguments, boost::optional<ast::lambda> lambda);

        /**
         * The name of the method.
         */
        name method;

        /**
         * The arguments to the method.
         */
        boost::optional<std::vector<expression>> arguments;

        /**
         * The optional lambda to the method.
         */
        boost::optional<ast::lambda> lambda;

        /**
         * Gets the position of the expression.
         */
        lexer::position const& position() const;
    };

    /**
     * Stream insertion operator for AST method call expression.
     * @param os The output stream to write the method call expression to.
     * @param expr The method call expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, method_call_expression const& expr);

}}  // puppet::ast
