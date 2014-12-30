/**
 * @file
 * Declares the AST parameter.
 */
#pragma once

#include "../lexer/lexer.hpp"
#include "expression.hpp"
#include "type.hpp"
#include "variable.hpp"
#include <boost/optional.hpp>
#include <vector>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents the type of an AST parameter.
     */
    struct parameter_type
    {
        /**
         * Default constructor for parameter_type.
         */
        parameter_type();

        /**
         * Constructs a parameter type with the given type and type expressions.
         * @param type The type of the parameter.
         * @param expressions The optional type expressions.
         */
        parameter_type(ast::type type, boost::optional<std::vector<expression>> expressions);

        /**
         * Gets the type of the parameter.
         * @return Returns the type of the parameter.
         */
        ast::type const& type() const;

        /**
         * Gets the type of the parameter.
         * @return Returns the type of the parameter.
         */
        ast::type& type();

        /**
         * Gets the optional type expressions.
         * @return Returns the optional type expressions.
         */
        boost::optional<std::vector<expression>> const& expressions() const;

        /**
         * Gets the optional type expressions.
         * @return Returns the optional type expressions.
         */
        boost::optional<std::vector<expression>>& expressions();

        /**
         * Gets the position of the parameter type.
         * @return Returns the position of the parameter type.
         */
        lexer::token_position const& position() const;

     private:
        struct type _type;
        boost::optional<std::vector<expression>> _expressions;
    };

    /**
     * Stream insertion operator for AST parameter type.
     * @param os The output stream to write the parameter type to.
     * @param type The parameter type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, parameter_type const& type);

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
        parameter(boost::optional<parameter_type> type, bool captures, struct variable variable, boost::optional<expression> default_value);

        /**
         * Gets the optional type of the parameter.
         * @return Returns the optional type of the parameter.
         */
        boost::optional<parameter_type> const& type() const;

        /**
         * Gets the optional type of the parameter.
         * @return Returns the optional type of the parameter.
         */
        boost::optional<parameter_type>& type();

        /**
         * Determines if the parameter captures the remaining arguments.
         * @return Returns true if the parameter captures the remaining arguments or false if not.
         */
        bool captures() const;

        /**
         * Determines if the parameter captures the remaining arguments.
         * @return Returns true if the parameter captures the remaining arguments or false if not.
         */
        bool& captures();

        /**
         * Gets the variable of the parameter.
         * @return Returns the variable of the parameter.
         */
        ast::variable const& variable() const;

        /**
         * Gets the variable of the parameter.
         * @return Returns the variable of the parameter.
         */
        ast::variable& variable();

        /**
         * Gets the optional default value expression of the parameter.
         * @return Returns the optional default value expression of the parameter.
         */
        boost::optional<expression> const& default_value() const;

        /**
         * Gets the optional default value expression of the parameter.
         * @return Returns the optional default value expression of the parameter.
         */
        boost::optional<expression>& default_value();

        /**
         * Gets the position of the parameter.
         * @return Returns the position of the parameter.
         */
        lexer::token_position const& position() const;

     private:
        boost::optional<parameter_type> _type;
        bool _captures;
        struct variable _variable;
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
