/**
 * @file
 * Declares the compile command.
 */
#pragma once

#include "parse.hpp"
#include "../../facts/provider.hpp"
#include <memory>

namespace puppet { namespace options { namespace commands {

    /**
     * Represents the compile command.
     */
    struct compile : parse
    {
        // Use the base constructor
        using parse::parse;

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
         * Gets the facts provider from the given options.
         * @param options The options to get the facts provider from.
         * @return Returns the facts provider.
         */
        std::shared_ptr<facts::provider> get_facts(boost::program_options::variables_map const& options) const;

        /**
         * Gets the node name from the given parsed options.
         * @param options The parsed options.
         * @param facts The facts provider to fallback to.
         * @return Returns the node name.
         */
        std::string get_node(boost::program_options::variables_map const& options, facts::provider& facts) const;

        /**
         * Gets the graph file from the given parsed options.
         * @param options The parsed options.
         * @return Returns the graph file.
         */
        std::string get_graph_file(boost::program_options::variables_map const& options) const;

        /**
         * The facts option name.
         */
        static char const* const FACTS_OPTION;
        /**
         * The facts option full name.
         */
        static char const* const FACTS_OPTION_FULL;
        /**
         * The facts option description.
         */
        static char const* const FACTS_DESCRIPTION;
        /**
         * The graph file option name.
         */
        static char const* const GRAPH_FILE_OPTION;
        /**
         * The graph file option full name.
         */
        static char const* const GRAPH_FILE_OPTION_FULL;
        /**
         * The graph file option description.
         */
        static char const* const GRAPH_FILE_DESCRIPTION;
        /**
         * The node option name.
         */
        static char const* const NODE_OPTION;
        /**
         * The node option full name.
         */
        static char const* const NODE_OPTION_FULL;
        /**
         * The node option description.
         */
        static char const* const NODE_DESCRIPTION;
        /**
         * The output option description.
         */
        static char const* const OUTPUT_DESCRIPTION;
    };

}}}  // namespace puppet::options::commands
