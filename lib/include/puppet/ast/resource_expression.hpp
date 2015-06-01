/**
 * @file
 * Declares the AST resource expression.
 */
#pragma once

#include "expression.hpp"
#include <vector>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents the AST resource attribute operator.
     */
    enum class attribute_operator
    {
        /**
         * No operator.
         */
        none,
        /**
         * The assignment (=>) operator.
         */
        assignment,
        /**
         * The append (+>) operator.
         */
        append
    };

    /**
     * Stream insertion operator for AST attribute operator.
     * @param os The output stream to write the operator to.
     * @param op The operator to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, attribute_operator op);

    /**
     * Represents an AST resource attribute expression.
     */
    struct attribute_expression
    {
        /**
         * Default constructor for attribute expression.
         */
        attribute_expression();

        /**
         * Constructs an attribute expression with the given name, operator, and expression.
         * @param attribute_name The name of the attribute.
         * @param op The attribute operator.
         * @param value The attribute's value expression.
         */
        attribute_expression(ast::name attribute_name, attribute_operator op, expression value);

        /**
         * Gets the name of the attribute.
         * @return Returns the name of the attribute.
         */
        ast::name const& name() const;

        /**
         * Gets the attribute operator.
         * @return Returns the attribute operator.
         */
        attribute_operator op() const;

        /**
         * Gets the attribute's value expression.
         * @return Returns the attribute's value expression.
         */
        expression const& value() const;

        /**
         * Gets the position of the attribute expression.
         * @return Returns the position of the attribute expression.
         */
        lexer::position const& position() const;

     private:
        ast::name _name;
        attribute_operator _op;
        expression _value;
    };

    /**
     * Stream insertion operator for AST attribute expression.
     * @param os The output stream to write the attribute expression to.
     * @param attribute The attribute expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, attribute_expression const& attribute);

    /**
     * Represents an AST resource body.
     */
    struct resource_body
    {
        /**
         * Default constructor for resource_body.
         */
        resource_body();

        /**
         * Constructs a resource body with the given title and optional attributes.
         * @param title The title expression of the resource.
         * @param attributes The optional attributes.
         */
        resource_body(expression title, boost::optional<std::vector<attribute_expression>> attributes);

        /**
         * Gets the resource body's title expression.
         * @return Returns the resource body's title expression.
         */
        expression const& title() const;

        /**
         * Gets the optional attributes of the resource.
         * @return Returns optional the attributes of the resource.
         */
        boost::optional<std::vector<attribute_expression>> const& attributes() const;

        /**
         * Gets the position of the resource body.
         * @return Returns the position of the resource body.
         */
        lexer::position const& position() const;

     private:
        expression _title;
        boost::optional<std::vector<attribute_expression>> _attributes;
    };

    /**
     * Stream insertion operator for AST resource body.
     * @param os The output stream to write the resource body to.
     * @param body The resource body to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource_body const& body);

    /**
     * Represents the status of a resource.
     */
    enum class resource_status
    {
        /**
         * The resource is realized.
         */
        realized,
        /**
         * The resource is virtualized.
         */
        virtualized,
        /**
         * The resource is exported.
         */
        exported
    };

    /**
     * Represents an AST resource expression.
     */
    struct resource_expression
    {
        /**
         * Default constructor for resource_expression.
         */
        resource_expression();

        /**
         * Constructs a resource expression with the given type and resource bodies.
         * @param type The type of the resource being defined.
         * @param bodies The resource bodies being defined.
         * @param status The resource status.
         */
        resource_expression(ast::name type, std::vector<resource_body> bodies, resource_status status = resource_status::realized);

        /**
         * Gets the type expression of the resource being defined.
         * @return Returns the type expression of the resource being defined.
         */
        ast::name const& type() const;

        /**
         * Gets the resource bodies that are being defined.
         * @return Returns the resource bodies that are being defined.
         */
        std::vector<resource_body> bodies() const;

        /**
         * Gets the status of the resource.
         * @return Returns the status of the resource.
         */
        resource_status status() const;

        /**
         * Gets the position of the resource expression.
         * @return Returns the position of the resource expression.
         */
        lexer::position const& position() const;

     private:
        ast::name _type;
        std::vector<resource_body> _bodies;
        resource_status _status;
    };

    /**
     * Stream insertion operator for AST resource expression.
     * @param os The output stream to write the resource expression to.
     * @param expression The resource expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource_expression const& expression);

}}  // namespace puppet::ast
