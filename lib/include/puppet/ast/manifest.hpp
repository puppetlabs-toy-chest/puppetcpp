/**
 * @file
 * Declares the AST manifest.
 */
#pragma once

#include "expression_def.hpp"
#include <boost/optional.hpp>
#include <iostream>
#include <vector>

namespace puppet { namespace ast {

    /**
     * Represents the AST manifest.
     */
    struct manifest
    {
        /**
         * Default constructor for manifest.
         */
        manifest();

        /**
         * Constructs a manifest with the given optional list of expressions that comprise the body.
         * @param body The body of the manifest.
         * @param end The ending token position for the manifest (applicable to interpolation).
         */
        manifest(boost::optional<std::vector<expression>> body, lexer::position end = lexer::position());

        /**
         * Gets the expressions that make up the body of the manifest.
         * @return Returns the expressions that make up the body of the manifest.
         */
        boost::optional<std::vector<expression>> const& body() const;

        /**
         * Gets the end token position for interpolation.
         * @return Returns the end token position for interpolation or nullptr if not interpolated.
         */
        lexer::position const& end() const;

    private:
        boost::optional<std::vector<expression>> _body;
        lexer::position _end;
    };

    /**
     * Stream insertion operator for AST manifest.
     * @param os The output stream to write the manifest to.
     * @param manifest The manifest to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::manifest const& manifest);

}}  // namespace puppet::ast
