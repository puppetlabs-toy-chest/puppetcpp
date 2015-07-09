/**
 * @file
 * Declares the compiler node.
 */
#pragma once

#include "exceptions.hpp"
#include "../logging/logger.hpp"
#include "../runtime/catalog.hpp"
#include "environment.hpp"
#include <exception>
#include <functional>
#include <set>

namespace puppet { namespace compiler {

    /**
     * Represents a compilation node.
     */
    struct node
    {
        /**
         * Constructs a compilation node.
         * @param name The name of the node.
         * @param environment The environment for the node.
         */
        node(std::string const& name, compiler::environment& environment);

        /**
         * Gets the display name of the node.
         * @return Returns the display name of the node.
         */
        std::string const& name() const;

        /**
         * Gets the node's environment.
         * @return Returns the node's environment.
         */
        compiler::environment& environment();

        /**
         * Compiles a manifest into a catalog for this node.
         * @param logger The logger to use.
         * @param path The path of the manifest file being compiled.
         * @return Returns the compiled catalog for the node.
         */
        runtime::catalog compile(logging::logger& logger, std::string const& path);

        /**
         * Calls the given callback for each name associated with the node.
         * @param callback The callback to call for each name.
         */
        void each_name(std::function<bool(std::string const&)> const& callback) const;

     private:
        std::set<std::string> _names;
        compiler::environment& _environment;
    };

}}  // puppet::compiler
