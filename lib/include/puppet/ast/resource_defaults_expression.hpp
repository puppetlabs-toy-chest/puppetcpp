/**
 * @file
 * Declares the AST resource defaults expression.
 */
#pragma once

#include "resource_expression.hpp"
#include <vector>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents an AST resource defaults expression.
     */
    struct resource_defaults_expression
    {
        /**
         * Default constructor for resource defaults expression.
         */
        resource_defaults_expression();

        /**
         * Constructs a resource defaults expression given the resource type and optional list of attributes.
         * @param type The type of the resource being defaulted.
         * @param attributes The optional attributes being defaulted.
         */
        resource_defaults_expression(ast::type type, boost::optional<std::vector<attribute_expression>> attributes);

        /**
         * The type of the resource default.
         */
        ast::type type;

        /**
         * The optional attributes being defaulted.
         */
        boost::optional<std::vector<attribute_expression>> attributes;

        /**
         * Gets the position of the expression.
         */
        lexer::position const& position() const;
    };

    /**
     * Stream insertion operator for AST resource defaults expression.
     * @param os The output stream to write the resource defaults expression to.
     * @param expr The resource defaults expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource_defaults_expression const& expr);

}}  // namespace puppet::ast
