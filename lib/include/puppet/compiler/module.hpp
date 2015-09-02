/**
 * @file
 * Declares a Puppet module.
 */
#pragma once

#include "../logging/logger.hpp"
#include <string>
#include <unordered_map>

namespace puppet { namespace compiler {

    /**
     * Represents a Puppet module.
     */
    struct module
    {
        /**
         * Constructs a Puppet module.
         * @param logger The logger to use.
         * @param name The name of the module (e.g. foo).
         * @param base The base path of the module (e.g. .../modules/foo).
         */
        module(logging::logger& logger, std::string name, std::string base);

        /**
         * Gets the logger used for logging messages.
         * @return Returns the logger used for logging messages.
         */
        logging::logger& logger();

        /**
         * Gets the name of the module.
         * @return Returns the name of the node.
         */
        std::string const& name() const;

        /**
         * Gets the base directory of the module.
         * @return Returns the base directory of the module.
         */
        std::string const& base() const;

        /**
         * Finds a manifest based on the qualified name (e.g. foo::bar => ./foo/bar.pp).
         * @param name The qualified name.
         * @return Returns the path to the manifest or an empty string if the manifest does not exist.
         */
        std::string find_manifest(std::string const& name) const;

        /**
         * Determines if the given name is a valid module name.
         * @param name The name to validate.
         * @return Returns true if the name is a valid module name or false if it is not.
         */
        static bool is_valid_name(std::string const& name);

    private:
        logging::logger& _logger;
        std::string _name;
        std::string _base;
    };

    /**
     * Loads all modules from the given directories.
     * @param logger The logger to use.
     * @param directories The directories to load the modules from.
     * @return Returns a map between the module name and the module.
     */
    std::unordered_map<std::string, module> load_modules(logging::logger& logger, std::vector<std::string> const& directories);

}}  // puppet::compiler
