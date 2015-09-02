/**
 * @file
 * Declares the compilation environment.
 */
#pragma once

#include "module.hpp"
#include "../logging/logger.hpp"
#include <string>
#include <vector>

namespace puppet { namespace compiler {

    /**
     * Represents a compilation environment.
     */
    struct environment
    {
        /**
         * Constructs a new environment.
         * @param logger The logger to use.
         * @param name The name of the environment (e.g. 'production').
         * @param base The base path for the environment.
         */
        environment(logging::logger& logger, std::string name, std::string base);

        /**
         * Gets the logger used for logging messages.
         * @return Returns the logger used for logging messages.
         */
        logging::logger& logger();

        /**
         * Gets the name of the environment.
         * @return Returns the name of the environment.
         */
        std::string const& name() const;

        /**
         * Gets the base directory of the environment.
         * @return Returns the base directory of the environment.
         */
        std::string const& base() const;

        /**
         * Finds the paths to the environment's manifests in alphabetical order.
         * @return Returns the paths to the environment's manifests in alphabetical order.
         */
        std::vector<std::string> find_manifests() const;

        /**
         * Finds a module by name.
         * @param name The module name to find.
         * @return Returns a pointer to the module or nullptr if the module does not exist.
         */
        module const* find_module(std::string const& name) const;

     private:
        logging::logger& _logger;
        std::string _name;
        std::string _base;
        std::unordered_map<std::string, module> _modules;
    };

}}  // puppet::compiler
