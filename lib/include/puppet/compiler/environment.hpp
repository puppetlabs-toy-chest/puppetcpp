/**
 * @file
 * Declares the compilation environment.
 */
#pragma once

#include <string>

namespace puppet { namespace compiler {

    /**
     * Represents a compilation environment.
     */
    struct environment
    {
        /**
         * Constructs a new environment.
         * @param name The name of the environment (e.g. 'production').
         * @param base The base path for the environment.
         */
        environment(std::string name, std::string base);

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

     private:
        std::string _name;
        std::string _base;
    };

}}  // puppet::compiler
