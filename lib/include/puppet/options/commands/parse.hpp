/**
 * @file
 * Declares the parse command.
 */
#pragma once

#include "../command.hpp"
#include "../../compiler/settings.hpp"

namespace puppet { namespace options { namespace commands {

    /**
     * Represents the parse command.
     */
    struct parse : command
    {
        // Use the base constructor
        using command::command;

        /**
         * Gets the name of the command.
         * @return Returns the name of the command.
         */
        char const* name() const override;

        /**
         * Gets the short description of the command.
         * @return Returns the short description of the command.
         */
        char const* description() const override;

        /**
         * Gets the summary of the command.
         * @return Returns the summary of the command.
         */
        char const* summary() const override;

        /**
         * Gets the command's argument format string, i.e. "[foo]".
         * @return Returns the command's argument format string.
         */
        char const* arguments() const override;

        /**
         * Creates the command's options.
         * @return Returns the command's options.
         */
        boost::program_options::options_description create_options() const override;

     protected:
        /**
         * Creates the command's hidden options.
         * @return Returns the command's hidden options.
         */
        boost::program_options::options_description create_hidden_options() const override;

        /**
         * Creates the command's positional options.
         * @return Returns the command's positional options.
         */
        boost::program_options::positional_options_description create_positional_options() const override;

        /**
         * Creates an executor for the given parsed options.
         * @param options The parsed options.
         * @return Returns the command executor.
         */
        executor create_executor(boost::program_options::variables_map const& options) const override;

        /**
         * Gets the manifests from the given parsed options.
         * @param options The parsed options.
         * @return Returns the manifests.
         */
        std::vector<std::string> get_manifests(boost::program_options::variables_map const& options) const;

        /**
         * Gets the output file from the given parsed options.
         * @param options The parsed options.
         * @return Returns the output file.
         */
        std::string get_output_file(boost::program_options::variables_map const& options) const;

        /**
         * Creates compiler settings based on the given parsed options.
         * @param options The parsed options.
         * @return Returns the compiler settings.
         */
        compiler::settings create_settings(boost::program_options::variables_map const& options) const;

        /**
         * The code directory option name.
         */
        static char const* const CODE_DIRECTORY_OPTION;
        /**
         * The code directory option description.
         */
        static char const* const CODE_DIRECTORY_DESCRIPTION;
        /**
         * The environment option name.
         */
        static char const* const ENVIRONMENT_OPTION;
        /**
         * The environment option full name.
         */
        static char const* const ENVIRONMENT_OPTION_FULL;
        /**
         * The environment option description.
         */
        static char const* const ENVIRONMENT_DESCRIPTION;
        /**
         * The environment path option name.
         */
        static char const* const ENVIRONMENT_PATH_OPTION;
        /**
         * The environment path option description.
         */
        static char const* const ENVIRONMENT_PATH_DESCRIPTION;
        /**
         * The log level option description.
         */
        static char const* const LOG_LEVEL_DESCRIPTION;
        /**
         * The manifests option name.
         */
        static char const* const MANIFESTS_OPTION;
        /**
         * The module path option name.
         */
        static char const* const MODULE_PATH_OPTION;
        /**
         * The module path option description.
         */
        static char const* const MODULE_PATH_DESCRIPTION;
        /**
        * The output option name.
        */
        static char const* const OUTPUT_OPTION;
        /**
         * The output option full name.
         */
        static char const* const OUTPUT_OPTION_FULL;
        /**
         * The output option description.
         */
        static char const* const OUTPUT_DESCRIPTION;
    };

}}}  // namespace puppet::options::commands
