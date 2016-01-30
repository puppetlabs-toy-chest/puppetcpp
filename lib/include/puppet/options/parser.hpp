/**
 * @file
 * Declares the command line options parser.
 */
#pragma once

#include "command.hpp"
#include "../cast.hpp"
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <exception>

namespace puppet { namespace options {

    /**
     * Exception for option errors.
     */
    struct option_exception : std::runtime_error
    {
        /**
         * Creates an option exception.
         * @param message The exception message.
         * @param command The command associated with the exception or nullptr if no command.
         */
        explicit option_exception(std::string const& message, options::command const* command = nullptr);

        /**
         * Gets the associated command.
         * @return Returns the associated command or nullptr if there is no associated command.
         */
        options::command const* command() const;

     private:
        options::command const* _command;
    };

    /**
     * Responsible for parsing command line options.
     */
    struct parser
    {
        /**
         * Adds a command to the parser.
         * @tparam Command The command type.
         * @tparam Args The additional argument types.
         * @param args The additional arguments to the command's constructor.
         */
        template <typename Command, typename... Args>
        void add(Args&&... args)
        {
            auto command = std::make_unique<Command>(*this, std::forward<Args>(args)...);
            auto name = command->name();
            _commands.emplace(rvalue_cast(name), rvalue_cast(command));
        }

        /**
         * Finds a command by name.
         * @param name The command name.
         * @return Returns the command if found or nullptr if not found.
         */
        command const* find(std::string const& name) const;

        /**
         * Enumerates each command in the parser.
         * @param callback The callback to call for each command.
         */
        void each(std::function<bool(command const&)> const& callback) const;

        /**
         * Parses command line arguments into a command executor.
         * @param arguments The command line arguments.
         * @return Returns the parsed command executor.
         */
        executor parse(std::vector<std::string> const& arguments) const;

     private:
        std::map<std::string, std::unique_ptr<command>> _commands;
    };

}}  // namespace puppet::options
