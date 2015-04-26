/**
 * @file
 * Declares the runtime evaluation context.
 */
#pragma once

#include "scope.hpp"
#include "../logging/logger.hpp"
#include "../lexer/token_position.hpp"
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <functional>

namespace puppet { namespace runtime {

    /**
     * Represents the runtime evaluation context.
     */
    struct context
    {
        /**
         * Constructs an evaluation context.
         * @param logger The logger to use.
         * @param warn The warning callback to use.
         */
        context(logging::logger& logger, std::function<void(lexer::token_position const&, std::string const&)> warn = nullptr);

        /**
         * Pushes an ephemeral scope.
         */
        void push();

        /**
         * Pushes a new scope onto the evaluation context.
         * @param name The name of the scope.
         * @param parent_name The name of the scope's parent scope.
         * @return Returns true if the scope was pushed or false if not (scope with the given name already exists).
         */
        bool push(std::string name, std::string const& parent_name = std::string());

        /**
         * Pops the current scope.
         * @return Returns true if the scope was popped or false if not (already at top scope).
         */
        bool pop();

        /**
         * Lookup a variable by name.
         * @name The name of the variable to lookup.
         * @return Returns the value of the variable or nullptr if the variable is not defined.
         */
        values::value const* lookup(std::string const& name) const;

        /**
         * Gets the current scope.
         * @return Returns the current scope.
         */
        scope const& current() const;

        /**
         * Gets the current scope.
         * @return Returns the current scope.
         */
        scope& current();

        /**
         * Gets the logger.
         * @return Returns the logger.
         */
        logging::logger& logger();

        /**
         * Emits a warning with the given position and message.
         * @param position The position of the warning.
         * @param message The warning message.
         */
        void warn(lexer::token_position const& position, std::string const& message) const;

    private:
        logging::logger& _logger;
        std::unordered_map<std::string, std::shared_ptr<scope>> _scopes;
        std::stack<std::shared_ptr<scope>> _stack;
        std::shared_ptr<scope> _top;
        std::function<void(lexer::token_position const&, std::string const&)> _warn;
    };

    /**
     * Helper for creating an ephemeral scope.
     */
    struct ephemeral_scope
    {
        /**
         * Constructs an ephemeral scope.
         * @param ctx The current evaluation context.
         */
        explicit ephemeral_scope(context& ctx);

        /**
         * Destructs the ephemeral scope.
         */
        ~ephemeral_scope();

     private:
        context& _ctx;
    };

}}  // puppet::runtime
