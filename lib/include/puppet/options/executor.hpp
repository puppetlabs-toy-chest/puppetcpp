/**
 * @file
 * Declares the command executor.
 */
#pragma once

#include <functional>

namespace puppet { namespace options {

    // Forward declaration of command
    struct command;

    /**
     * Represents the command executor.
     */
    struct executor
    {
        /**
         * The execution callback type.
         */
        using callback_type = std::function<int()>;

        /**
         * Constructor for executor.
         * @param command The associated command.
         * @param callback The callback to call for command execution.
         */
        executor(options::command const& command, callback_type callback);

        /**
         * Gets the associated command.
         * @return Returns the associated command.
         */
        options::command const& command() const;

        /**
         * Executes the command.
         * @return Returns the command exit code (e.g. EXIT_SUCCESS or EXIT_FAILURE).
         */
        int execute() const;

     private:
        options::command const& _command;
        callback_type _callback;
    };

}}  // namespace puppet::options
