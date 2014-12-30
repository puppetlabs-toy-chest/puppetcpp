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
         */
        explicit manifest(boost::optional<std::vector<expression>> body);

        /**
         * Gets the expressions that make up the body of the manifest.
         * @return Returns the expressions that make up the body of the manifest.
         */
        boost::optional<std::vector<expression>> const& body() const;

        /**
         * Gets the expressions that make up the body of the manifest.
         * @return Returns the expressions that make up the body of the manifest.
         */
        boost::optional<std::vector<expression>>& body();

    private:
        boost::optional<std::vector<expression>> _body;
    };

    /**
     * Stream insertion operator for AST manifest.
     * @param os The output stream to write the manifest to.
     * @param manifest The manifest to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::manifest const& manifest);

}}  // namespace puppet::ast
