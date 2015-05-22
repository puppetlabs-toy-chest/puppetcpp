/**
 * @file
 * Declares the AST function call expression.
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
     * Represents an AST function call expression.
    */
    struct function_call_expression
    {
        /**
         * Default constructor for function_call_expression.
         */
        function_call_expression();

        /**
         * Constructs a function call expression with the given function name, optinoal arguments, and optional lambda.
         * @param function The name of the function being called.
         * @param arguments The optional argument expressions to the function.
         * @param lambda The optional lambda to the function.
         */
        function_call_expression(name function, boost::optional<std::vector<expression>> arguments, boost::optional<ast::lambda> lambda);

        /**
         * The name of the function.
         */
        name function;

        /**
         * The arguments to the function.
         */
        boost::optional<std::vector<expression>> arguments;

        /**
         * The optional lambda to the function.
         */
        boost::optional<ast::lambda> lambda;

        /**
         * Gets the position of the expression.
         */
        lexer::position const& position() const;
    };

    /**
     * Stream insertion operator for AST function call expression.
     * @param os The output stream to write the function call expression to.
     * @param expr The function call expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, function_call_expression const& expr);

}}  // puppet::ast
