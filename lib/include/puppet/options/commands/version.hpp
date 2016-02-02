/**
 * @file
 * Declares the version command.
 */
#pragma once

#include "../command.hpp"
#include <iostream>

namespace puppet { namespace options { namespace commands {

    /**
     * Represents the version command.
     */
    struct version : command
    {
        /**
         * Constructor for version.
         * @param parser The associated options parser.
         * @param stream The stream to write the version to.
         */
        explicit version(options::parser const& parser, std::ostream& stream = std::cout);

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

     protected:
        /**
         * Creates an executor for the given parsed options.
         * @param options The parsed options.
         * @return Returns the command executor.
         */
        executor create_executor(boost::program_options::variables_map const& options) const override;

     private:
        std::ostream& _stream;
    };

}}}  // namespace puppet::options::commands
