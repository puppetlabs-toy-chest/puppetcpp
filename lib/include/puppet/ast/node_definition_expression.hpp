/**
 * @file
 * Declares the AST node definition expression.
 */
#pragma once

#include "../lexer/position.hpp"
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
         * Constructs a default hostname.
         * @param defaulted The default value.
         */
        explicit hostname(ast::defaulted defaulted);

        /**
         * Constructs a hostname from a list of name or number parts.
         * @param parts The parts that make up the hostname.
         */
        explicit hostname(std::vector<boost::variant<name, bare_word, number>> const& parts);

        /**
         * Constructs a hostname from the given AST string.
         * @param name The hostname as a string.
         */
        explicit hostname(ast::string name);

        /**
         * Constructs a hostname from the given AST regex.
         * @param name The hostname as a regex.
         */
        explicit hostname(ast::regex name);

        /**
         * The position of the hostname.
         */
        lexer::position position;

        /**
         * The value of the hostname.
         */
        std::string value;

        /**
         * Whether or not the value of the hostname is a regex.
         */
        bool regex;
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
        node_definition_expression(lexer::position position, std::vector<hostname> names, boost::optional<std::vector<expression>> body);

        /**
         * The position of the node definition.
         */
        lexer::position position;

        /**
         * The hostnames of the node definition.
         */
        std::vector<hostname> names;

        /**
         * The body of the node definition.
         */
        boost::optional<std::vector<expression>> body;
    };

    /**
     * Stream insertion operator for AST node definition expression.
     * @param os The output stream to write the node definition expression to.
     * @param expr The node definition expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, node_definition_expression const& expr);

}}  // namespace puppet::ast
