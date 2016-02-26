/**
 * @file
 * Declares the default options.
 */
#pragma once

#include <string>
#include <vector>

namespace puppet { namespace options {

    /**
     * Represents platform-specific option defaults.
     */
    struct defaults
    {
        /**
         * Gets the default code directory for the current platform.
         * @return Returns the default code directory.
         */
        static std::string code_directory();

        /**
         * Gets the default environment path for the current platform.
         * @return Returns the default environment path.
         */
        static std::string environment_path();

        /**
         * Gets the default module path for the current platform.
         * @return Returns the default module path.
         */
        static std::string module_path();
    };

}}  // namespace puppet::options
