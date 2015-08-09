/**
 * @file
 * Declares the compiler settings.
 */
#pragma once

#include "../facts/provider.hpp"
#include "../logging/logger.hpp"
#include <string>
#include <vector>
#include <memory>

namespace puppet { namespace compiler {

    /**
     * Represents the settings for the Puppet compiler.
     */
    struct settings
    {
        /**
         * Constructs the default settings.
         */
        settings();

        /**
         * Constructs a settings from command line arguments.
         * @param argc The number of arguments in the given argument vector.
         * @param argv The vector of arguments.
         */
        settings(int argc, char const* argv[]);

        /**
         * Gets the code directory ($codedir).
         * Defaults to a platform-specific directory.
         * @return Returns the code directory.
         */
        std::string const& code_directory() const;

        /**
         * Gets the environment name.
         * Defaults to "production".
         * @return Returns the environment name.
         */
        std::string const& environment() const;

        /**
         * Gets the environment directory.
         * @return Returns the environment directory.
         */
        std::string const& environment_directory() const;

        /**
         * Gets the directories to search for global modules.
         * @return Returns the directories to search for global modules.
         */
        std::vector<std::string> const& module_directories() const;

        /**
         * Gets the manifests to compile.
         * Defaults to the manifest for the environment or the root site.pp.
         * @return Returns the manifests to compile.
         */
        std::vector<std::string> const& manifests() const;

        /**
         * Gets the name of the node.
         * Defaults to the 'fqdn' fact or calculated from the hostname and domain facts.
         * @return Returns the node name.
         */
        std::string const& node_name() const;

        /**
         * Gets the path to the output file.
         * @return Returns the path to the output file.
         */
        std::string const& output_file() const;

        /**
         * Gets the facts provider to use.
         * Defaults to the facter facts provider.
         * @return Returns the facts provider to use.
         */
        std::shared_ptr<facts::provider> const& facts() const;

        /**
         * Gets the log level to use.
         * Defaults to notice.
         * @return Returns the log level to use.
         */
        logging::level log_level() const;

        /**
         * Gets whether or not the help should be displayed.
         * Defaults to false. If true, no other option will be valid.
         * @return Returns whether or not the help should be displayed.
         */
        bool show_help() const;

        /**
         * Gets whether or not the version should be displayed.
         * Defaults to false. If true, no other option will be valid.
         * @return Returns whether or not the version should be displayed.
         */
        bool show_version() const;

        /**
         * Prints the compiler's usage.
         */
        static void print_usage();

        /**
         * Gets the default code directory for the current platform.
         * @return Returns the default code directory.
         */
        static std::string default_code_directory();

        /**
         * Gets the default environment directories for the current platform.
         * @return Returns the default environment directories.
         */
        static std::vector<std::string> default_environment_directories();

        /**
         * Gets the default global module directories for the current platform.
         * @return Returns the default global modules directories.
         */
        static std::vector<std::string> default_module_directories();

    private:
        void parse(int argc, char const* argv[]);

        std::string _code_directory;
        std::string _environment;
        std::string _environment_directory;
        std::vector<std::string> _module_directories;
        std::vector<std::string> _manifests;
        std::string _node_name;
        std::string _output_file;
        std::shared_ptr<facts::provider> _facts;
        logging::level _log_level;
        bool _show_help;
        bool _show_version;
    };

}}  // namespace puppet::compiler