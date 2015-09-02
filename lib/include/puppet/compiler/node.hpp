/**
 * @file
 * Declares the compiler node.
 */
#pragma once

#include "module.hpp"
#include "exceptions.hpp"
#include "settings.hpp"
#include "../runtime/catalog.hpp"
#include "../runtime/context.hpp"
#include "../logging/logger.hpp"
#include "environment.hpp"
#include <exception>
#include <functional>
#include <set>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>

namespace puppet { namespace compiler {

    // Forward declaration of compiler context
    struct context;

    /**
     * Represents a compilation node.
     */
    struct node
    {
        /**
         * Constructs a compilation node.
         * @param logger The logger to use.
         * @param name The name of the node.
         * @param module_directories The directories to load modules from.
         * @param environment The environment for the node.
         */
        node(
            logging::logger& logger,
            std::string const& name,
            std::vector<std::string> const& module_directories,
            compiler::environment& environment);

        /**
         * Gets the logger used for logging messages.
         * @return Returns the logger used for logging messages.
         */
        logging::logger& logger();

        /**
         * Gets the display name of the node.
         * @return Returns the display name of the node.
         */
        std::string const& name() const;

        /**
         * Gets the node's environment.
         * @return Returns the node's environment.
         */
        compiler::environment const& environment() const;

        /**
         * Compiles manifests into a catalog for this node.
         * @param settings The compiler settings.
         * @return Returns the compiled catalog for the node.
         */
        runtime::catalog compile(compiler::settings const& settings);

        /**
         * Calls the given callback for each name associated with the node.
         * @param callback The callback to call for each name.
         */
        void each_name(std::function<bool(std::string const&)> const& callback) const;

        /**
         * Finds a module by name.
         * @param name The module name to find.
         * @return Returns a pointer to the module or nullptr if the module does not exist.
         */
        module const* find_module(std::string const& name) const;

        /**
         * Loads a manifest from the module into the evaluation context.
         * @param evaluation_context The current evaluation context.
         * @param name The class or defined type name to find a manifest for (.e.g. foo::bar).
         */
        void load_manifest(runtime::context& evaluation_context, std::string const& name);

     private:
        static void create_initial_resources(
            runtime::context& evaluation_context,
            std::shared_ptr<compiler::context> const& compilation_context,
            compiler::settings const& settings);

        logging::logger& _logger;
        std::set<std::string> _names;
        compiler::environment& _environment;
        std::unordered_map<std::string, module> _modules;
        std::unordered_set<std::string> _loaded_manifests;
    };

}}  // puppet::compiler
