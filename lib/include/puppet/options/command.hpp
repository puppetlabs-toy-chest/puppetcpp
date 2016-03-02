/**
 * @file
 * Declares the base command.
 */
#pragma once

#include "executor.hpp"
#include "../logging/logger.hpp"
#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <vector>
#include <string>

namespace puppet { namespace options {

    // Forward declare the options parser
    struct parser;

    /**
     * Represents the base command.
     */
    struct command
    {
        /**
         * Constructor for command.
         * @param parser The associated options parser.
         */
        explicit command(options::parser const& parser);

        /**
         * Default destructor for command.
         */
        virtual ~command() = default;

        /**
         * Gets the name of the command.
         * @return Returns the name of the command.
         */
        virtual char const* name() const = 0;

        /**
         * Gets the short description of the command.
         * @return Returns the short description of the command.
         */
        virtual char const* description() const = 0;

        /**
         * Gets the summary of the command.
         * @return Returns the summary of the command.
         */
        virtual char const* summary() const = 0;

        /**
         * Gets the command's argument format string, i.e. "[foo]".
         * @return Returns the command's argument format string.
         */
        virtual char const* arguments() const;

        /**
         * Gets the associated options parser.
         * @return Returns the associated options parser.
         */
        options::parser const& parser() const;

        /**
         * Parses the given command arguments.
         * @param arguments The arguments to the command.
         * @return Returns the command executor.
         */
        executor parse(std::vector<std::string> const& arguments) const;

        /**
         * Creates the command's options.
         * @return Returns the command's options.
         */
        virtual boost::program_options::options_description create_options() const;

     protected:
        /**
         * Creates the command's hidden options.
         * @return Returns the command's hidden options.
         */
        virtual boost::program_options::options_description create_hidden_options() const;

        /**
         * Creates the command's positional options.
         * @return Returns the command's positional options.
         */
        virtual boost::program_options::positional_options_description create_positional_options() const;

        /**
         * Creates an executor for the given parsed options.
         * @param options The parsed options.
         * @return Returns the command executor.
         */
        virtual executor create_executor(boost::program_options::variables_map const& options) const = 0;

        /**
         * Gets the logging level from the given parsed options.
         * @param options The parsed options.
         * @return Returns the logging level.
         */
        logging::level get_level(boost::program_options::variables_map const& options) const;

        /**
         * Gets the colorization option from the given parsed options.
         * @param options The parsed options.
         * @return Returns the colorization option.
         */
        boost::optional<bool> get_colorization(boost::program_options::variables_map const& options) const;

        /**
         * The debug option name.
         */
        static char const* const DEBUG_OPTION;
        /**
         * The debug option full name.
         */
        static char const* const DEBUG_OPTION_FULL;
        /**
         * The debug option description.
         */
        static char const* const DEBUG_DESCRIPTION;
        /**
         * The color option name.
         */
        static char const* const COLOR_OPTION;
        /**
         * The color option description.
         */
        static char const* const COLOR_DESCRIPTION;
        /**
         * The help option name.
         */
        static char const* const HELP_OPTION;
        /**
         * The help option description.
         */
        static char const* const HELP_DESCRIPTION;
        /**
         * The log level option name.
         */
        static char const* const LOG_LEVEL_OPTION;
        /**
         * The log level option full name.
         */
        static char const* const LOG_LEVEL_OPTION_FULL;
        /**
         * The log level option description.
         */
        static char const* const LOG_LEVEL_DESCRIPTION;
        /**
         * The no color option name.
         */
        static char const* const NO_COLOR_OPTION;
        /**
         * The no color option description.
         */
        static char const* const NO_COLOR_DESCRIPTION;
        /**
         * The verbose option name.
         */
        static char const* const VERBOSE_OPTION;
        /**
         * The verbose option description.
         */
        static char const* const VERBOSE_DESCRIPTION;

     private:
        options::parser const& _parser;
    };

}}  // namespace puppet::options
