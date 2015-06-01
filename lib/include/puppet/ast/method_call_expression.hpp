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
         * Gets the method name.
         * @return Returns the method name.
         */
        name const& method() const;

        /**
         * Gets the optional argument expressions.
         * @return Returns the argument expressions.
         */
        boost::optional<std::vector<expression>> const& arguments() const;

        /**
         * Gets the optional lambda.
         * @return Returns the optional lambda.
         */
        boost::optional<ast::lambda> const& lambda() const;

        /**
         * Gets the position of the method call expression.
         * @return Returns the position of the method call expression.
         */
        lexer::position const& position() const;

     private:
        name _method;
        boost::optional<std::vector<expression>> _arguments;
        boost::optional<ast::lambda> _lambda;
    };

    /**
     * Stream insertion operator for AST method call expression.
     * @param os The output stream to write the method call expression to.
     * @param expr The method call expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, method_call_expression const& expr);

}}  // puppet::ast
