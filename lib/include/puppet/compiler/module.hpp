/**
 * @file
 * Declares a Puppet module.
 */
#pragma once

#include "finder.hpp"
#include <string>

namespace puppet { namespace compiler {

    // Forward declaration of environment.
    struct environment;

    /**
     * Represents a Puppet module.
     */
    struct module : finder
    {
        /**
         * Constructs a Puppet module.
         * @param environment The environment containing this module.
         * @param directory The directory for the module.
         * @param name The name of the module (e.g. foo).
         */
        module(compiler::environment& environment, std::string directory, std::string name);

        /**
         * Gets the environment containing the module.
         * @return Returns the environment containing the module.
         */
        compiler::environment const& environment() const;

        /**
         * Gets the name of the module.
         * @return Returns the name of the node.
         */
        std::string const& name() const;

        /**
         * Determines if the given name is a valid module name.
         * @param name The name to validate.
         * @return Returns true if the name is a valid module name or false if it is not.
         */
        static bool is_valid_name(std::string const& name);

    private:
        compiler::environment& _environment;
        std::string _name;
    };

}}  // puppet::compiler
