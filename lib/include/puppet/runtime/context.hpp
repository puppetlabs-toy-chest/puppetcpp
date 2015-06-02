/**
 * @file
 * Declares the Puppet evaluation context.
 */
#pragma once

#include "../lexer/position.hpp"
#include "../compiler/node.hpp"
#include "../logging/logger.hpp"
#include "scope.hpp"
#include "catalog.hpp"
#include <string>
#include <deque>
#include <unordered_map>
#include <functional>

namespace puppet { namespace runtime {

    /**
     * Represents the evaluation context.
     */
    struct context
    {
        /**
         * Constructs an evaluation context.
         * @param logger The logger to use for logging messages.
         * @param node The node being compiled.
         * @param catalog The catalog being compiled.
         * @param warning A function to call to output a warning at a given position.
         */
        context(logging::logger& logger, compiler::node& node, runtime::catalog& catalog, std::function<void(lexer::position const&, std::string const&)> const& warning);

        /**
         * Gets the logger used for logging messages.
         * @return Returns the logger used for logging messages.
         */
        logging::logger& logger();

        /**
         * Gets the logger used for logging messages.
         * @return Returns the logger used for logging messages.
         */
        logging::logger const& logger() const;

        /**
        * Gets the current compilation node.
        * @return Returns the current compilation node.
        */
        compiler::node& node();

        /**
         * Gets the current compilation node.
         * @return Returns the current compilation node.
         */
        compiler::node const& node() const;

        /**
         * Gets the catalog being compiled.
         * @return Returns the catalog being compiled.
         */
        runtime::catalog& catalog();

        /**
         * Gets the catalog being compiled.
         * @return Returns the catalog being compiled.
         */
        runtime::catalog const& catalog() const;

        /**
         * Gets the current scope.
         * @return Returns the current scope.
         */
        runtime::scope& scope();

        /**
         * Gets the current scope.
         * @return Returns the current scope.
         */
        runtime::scope const& scope() const;

        /**
         * Gets the top scope.
         * @return Returns the top scope.
         */
        runtime::scope& top();

        /**
         * Gets the top scope.
         * @return Returns the top scope.
         */
        runtime::scope const& top() const;

        /**
         * Adds a scope with the given name.
         * Note: if a scope of the same name already exists, the existing scope is returned unmodified.
         * @param name The name of the scope to add.
         * @param scope The scope to add.
         * @return Returns the scope.
         */
        runtime::scope* add_scope(std::string name, runtime::scope scope);

        /**
         * Finds a scope by name.
         * @param name The name of the scope to find.
         * @return Returns a pointer to the scope if found or nullptr if the scope is not found.
         */
        runtime::scope* find_scope(std::string const& name);

        /**
         * Pushes the given scope.
         * @param current The new current scope.
         */
        void push_scope(runtime::scope& current);

        /**
         * Pops the current scope.
         * @return Returns true if the scope was popped or false if not (i.e. already at top scope).
         */
        bool pop_scope();

        /**
         * Emits a warning with the given position and message.
         * @param position The position of the warning.
         * @param message The warning message.
         */
        void warn(lexer::position const& position, std::string const& message) const;

    private:
        logging::logger& _logger;
        compiler::node& _node;
        runtime::catalog& _catalog;
        std::unordered_map<std::string, runtime::scope> _scopes;
        std::deque<runtime::scope*> _scope_stack;
        std::function<void(lexer::position const&, std::string const&)> const& _warn;
    };

    /**
     * Helper for creating an ephemeral scope.
     */
    struct ephemeral_scope
    {
        /**
         * Constructs an ephemeral scope.
         * @param context The current evaluation context.
         */
        explicit ephemeral_scope(runtime::context& context);

        /**
         * Destructs the ephemeral scope.
         */
        ~ephemeral_scope();

    private:
        runtime::context& _context;
        scope _scope;
    };

}}  // namespace puppet::runtime
