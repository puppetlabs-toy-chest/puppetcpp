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
         * The body of the lambda.
         */
        boost::optional<std::vector<expression>> body;

        /**
         * The end position of the manifest (used in string interpolation).
         */
        lexer::position end;
    };

    /**
     * Stream insertion operator for AST manifest.
     * @param os The output stream to write the manifest to.
     * @param manifest The manifest to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::manifest const& manifest);

}}  // namespace puppet::ast
