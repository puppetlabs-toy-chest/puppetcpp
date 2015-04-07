/**
 * @file
 * Declares the AST node definition expression.
 */
#pragma once

#include "../lexer/token_position.hpp"
#include "expression.hpp"
#include <boost/optional.hpp>
#include <iostream>
#include <string>

namespace puppet { namespace ast {

    /**
     * Represents a node definition hostname.
     */
    struct hostname
    {
        /**
         * Default constructor for hostname.
         */
        hostname();

        /**
         * Constructs a hostname for the default hostname.
         * @param position The position of the default keyword.
         */
        explicit hostname(lexer::token_position position);

        /**
         * Constructs a hostname from a list of name or number parts.
         * @param parts The parts that make up the hostname.
         */
        explicit hostname(std::vector<boost::variant<name, number>> const& parts);

        /**
         * Constructs a hostname from the given AST string.
         * @param name The hostname string.
         */
        explicit hostname(string name);

        /**
         * Constructs a hostname from the given AST regex.
         * @param name The hostname regex.
         */
        explicit hostname(struct regex name);

        /**
         * Gets the value of the hostname.
         * @return Returns the value of the hostname.
         */
        std::string const& value() const;

        /**
         * Determines if the hostname is a regex.
         * @return Returns true if the hostname is a regex or false if it is a regular string.
         */
        bool regex() const;

        /**
         * Determines if the hostname is the default hostname.
         * @return Returns true if the hostname is the default hostname or false if it is not.
         */
        bool is_default() const;

        /**
         * Gets the position of the hostname.
         * @return Returns the position of the hostname.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        std::string _value;
        bool _regex;
    };

    /**
     * Stream insertion operator for AST hostname.
     * @param os The output stream to write the hostname to.
     * @param name The hostname to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, hostname const& name);

    /**
     * Represents the AST node definition expression.
     */
    struct node_definition_expression
    {
        /**
         * Default constructor for node_definition_expression.
         */
        node_definition_expression();

        /**
         * Constructs the node definition with the given position, hostnames, and optional body expressions.
         * @param position The position of the node definition expression.
         * @param names The hostnames for the node definition.
         * @param body The optional expressions that make up the body of the node definition.
         */
        node_definition_expression(lexer::token_position position, std::vector<hostname> names, boost::optional<std::vector<expression>> body);

        /**
         * Gets the list of hostnames for the node definition.
         * @return Returne the hostnames for the node definition.
         */
        std::vector<hostname> const& names() const;

        /**
         * Gets the optionl expressions that make up the definition'd body.
         * @return Returns the optional expressions that make up the definition's body.
         */
        boost::optional<std::vector<expression>> const& body() const;

        /**
         * Gets the position of the node definition expression.
         * @return Returns the position of the node definition expression.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        std::vector<hostname> _names;
        boost::optional<std::vector<expression>> _body;
    };

    /**
     * Stream insertion operator for AST node definition expression.
     * @param os The output stream to write the node definition expression to.
     * @param stmt The node definition expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, node_definition_expression const& stmt);

}}  // namespace puppet::ast
