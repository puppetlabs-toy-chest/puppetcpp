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
         * Gets the optional type of the parameter.
         * @return Returns the optional type of the parameter.
         */
        boost::optional<primary_expression> const& type() const;

        /**
         * Determines if the parameter captures the remaining arguments.
         * @return Returns true if the parameter captures the remaining arguments or false if not.
         */
        bool captures() const;

        /**
         * Gets the variable of the parameter.
         * @return Returns the variable of the parameter.
         */
        ast::variable const& variable() const;

        /**
         * Gets the optional default value expression of the parameter.
         * @return Returns the optional default value expression of the parameter.
         */
        boost::optional<expression> const& default_value() const;

        /**
         * Gets the position of the parameter.
         * @return Returns the position of the parameter.
         */
        lexer::position const& position() const;

     private:
        boost::optional<primary_expression> _type;
        bool _captures;
        ast::variable _variable;
        boost::optional<expression> _default_value;
    };

    /**
     * Stream insertion operator for AST parameter.
     * @param os The output stream to write the parameter to.
     * @param param The parameter to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, parameter const& param);

}}  // puppet::ast
