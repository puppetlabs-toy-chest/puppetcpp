/**
 * @file
 * Declares the syntax tree.
 */
#pragma once

#include "expression_def.hpp"
#include <boost/optional.hpp>
#include <iostream>
#include <vector>
#include <string>

namespace puppet { namespace ast {

    /**
     * Represents the syntax tree.
     */
    struct syntax_tree
    {
        /**
         * Default constructor for syntax tree.
         */
        syntax_tree() = default;

        /**
         * Constructs a syntax tree with the given optional list of expressions that comprise the body.
         * @param path The path of the file that was parsed.
         * @param body The body of the syntax tree.
         * @param end The ending token position for the syntax tree (applicable to interpolation).
         */
        syntax_tree(std::string path, boost::optional<std::vector<expression>> body, lexer::position end = lexer::position());

        /**
        * Gets the path to the file that was parsed.
        * @return Returns the path to the file that was pared.
        */
        std::string const& path() const;

        /**
         * Gets the expressions that make up the body of the syntax tree.
         * @return Returns the expressions that make up the body of the syntax tree.
         */
        boost::optional<std::vector<expression>> const& body() const;

        /**
         * Gets the end token position for interpolation.
         * @return Returns the end token position for interpolation or nullptr if not interpolated.
         */
        lexer::position const& end() const;

    private:
        std::string _path;
        boost::optional<std::vector<expression>> _body;
        lexer::position _end;
    };

    /**
     * Stream insertion operator for AST syntax tree.
     * @param os The output stream to write the syntax tree to.
     * @param tree The syntax tree to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::syntax_tree const& tree);

}}  // namespace puppet::ast
