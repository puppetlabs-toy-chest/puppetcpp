/**
 * @file
 * Declares the AST parameter.
 */
#pragma once

#include "../lexer/position.hpp"
#include "expression.hpp"
#include "type.hpp"
#include "variable.hpp"
#include <boost/optional.hpp>
#include <vector>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents an AST parameter.
     */
    struct parameter
    {
        /**
         * Default constructor for parameter.
         */
        parameter();

        /**
         * Constructs a parameter with the optional type, variable, and optional default value expression.
         * @param type The optional type of the parameter.
         * @param captures True if the variable captures the remaining arguments or false if not.
         * @param variable The variable of the parameter.
         * @param default_value The optional default value expression.
         */
        parameter(boost::optional<primary_expression> type, bool captures, ast::variable variable, boost::optional<expression> default_value);

        /**
         * The optional type of the parameter.
         */
        boost::optional<primary_expression> type;

        /**
         * Whether or not the parameter captures rest.
         */
        bool captures;

        /**
         * The variable of the parameter.
         */
        ast::variable variable;

        /**
         * The optional default value for the parameter.
         */
        boost::optional<expression> default_value;

        /**
         * Gets the position of the expression.
         * @return Returns the position of the expression.
         */
        lexer::position const& position() const;
    };

    /**
     * Stream insertion operator for AST parameter.
     * @param os The output stream to write the parameter to.
     * @param param The parameter to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, parameter const& param);

}}  // puppet::ast
