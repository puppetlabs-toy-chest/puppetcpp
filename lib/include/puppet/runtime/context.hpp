/**
 * @file
 * Declares the Puppet evaluation context.
 */
#pragma once

#include "../lexer/position.hpp"
#include "scope.hpp"
#include "catalog.hpp"
#include <string>
#include <memory>
#include <deque>
#include <unordered_map>

namespace puppet { namespace runtime {

    // Forward declaration of context.
    struct context;

    /**
     * Represents the evaluation context.
     */
    struct context
    {
        /**
         * Constructs an evaluation context.
         * @param catalog The catalog being compiled or nullptr if catalog expressions are not supported.
         */
        explicit context(runtime::catalog* catalog = nullptr);

        /**
         * Gets the catalog being compiled.
         * @return Returns the catalog being compiled or nullptr if catalog expressions are not supported.
         */
        runtime::catalog* catalog();

        /**
         * Gets the current scope.
         * @return Returns the current scope.
         */
        runtime::scope& scope();

        /**
         * Gets the top scope.
         * @return Returns the top scope.
         */
        runtime::scope& top_scope();

        /**
         * Gets the node scope.
         * @return Returns the node scope or nullptr if there currently is no node scope.
         */
        runtime::scope* node_scope();

        /**
         * Gets the node scope if there is one or returns the top scope if there isn't.
         * @return Returns the node scope if there is one or returns the top scope if there isn't.
         */
        runtime::scope& node_or_top();

        /**
         * Adds a scope to the evaluation context.
         * @param name The name of the scope to add.
         * @param display_name The display name of the scope.
         * @param parent The parent scope.
         * @return Returns the scope that was added or the scope with the same name that already exists.
         */
        runtime::scope& add_scope(std::string name, std::string display_name, runtime::scope* parent = nullptr);

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

     private:
        friend struct node_scope;

        runtime::catalog* _catalog;
        std::unordered_map<std::string, runtime::scope> _scopes;
        std::deque<runtime::scope*> _scope_stack;
        std::unique_ptr<runtime::scope> _node_scope;
    };

    /**
     * Helper for setting a local scope.
     */
    struct local_scope
    {
        /**
         * Constructs an local scope.
         * @param context The current evaluation context.
         * @param scope The scope to set in the evaluation context.  If nullptr, an ephemeral scope is created.
         */
        local_scope(runtime::context& context, runtime::scope* scope = nullptr);

        /**
         * Destructs the local scope.
         */
        ~local_scope();

    private:
        runtime::context& _context;
        scope _scope;
    };

    /**
     * Helper for setting a node scope.
     */
    struct node_scope
    {
        /**
         * Constructs a node scope with the given name.
         * @param context The current evaluation context.
         * @param name The node scope name.
         */
        node_scope(runtime::context& context, std::string name);

        /**
         * Destructs the node scope.
         */
        ~node_scope();

    private:
        runtime::context& _context;
    };

}}  // namespace puppet::runtime
