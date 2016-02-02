/**
 * @file
 * Declares the compile command.
 */
#pragma once

#include "../command.hpp"

namespace puppet { namespace options { namespace commands {

    /**
     * Represents the compile command.
     */
    struct compile : command
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
    };

}}}  // namespace puppet::options::commands
