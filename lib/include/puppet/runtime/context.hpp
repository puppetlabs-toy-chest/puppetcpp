/**
 * @file
 * Declares the Puppet evaluation context.
 */
#pragma once

#include "../lexer/position.hpp"
#include "../facts/provider.hpp"
#include "scope.hpp"
#include "catalog.hpp"
#include <string>
#include <memory>
#include <unordered_map>
#include <regex>

namespace puppet { namespace runtime {

    // Forward declaration of expression_evaluator.
    struct expression_evaluator;

    // Forward declaration of context.
    struct context;

    /**
     * Helper for creating a match scope in an evaluation context.
     */
    struct match_scope
    {
        /**
         * Constructs a match scope.
         * @param context The current evaluation context.
         */
        explicit match_scope(runtime::context& context);

        /**
         * Destructs a match scope.
         */
        ~match_scope();

     private:
        runtime::context& _context;
    };

    /**
     * Helper for setting a local scope.
     */
    struct local_scope : match_scope
    {
        /**
         * Constructs a local scope.
         * @param context The current evaluation context.
         * @param scope The scope to set in the evaluation context.  If nullptr, an ephemeral scope is created.
         */
        local_scope(runtime::context& context, std::shared_ptr<runtime::scope> scope = nullptr);

        /**
         * Destructs the local scope.
         */
        ~local_scope();

     private:
        runtime::context& _context;
    };

    /**
     * Helper for creating a node scope in an evaluation context.
     */
    struct node_scope
    {
        /**
         * Constructs a node scope.
         * @param context The current evaluation context.
         * @param name The name of the node scope.
         */
        node_scope(runtime::context& context, std::string name);

        /**
         * Destructs the node scope.
         */
        ~node_scope();

     private:
        runtime::context& _context;
    };

    /**
     * Represents the evaluation context.
     */
    struct context
    {
        /**
         * Constructs an evaluation context.
         * @param facts The facts provider to use for fact lookup.
         * @param catalog The catalog being compiled or nullptr if catalog expressions are not supported.
         */
        explicit context(std::shared_ptr<facts::provider> facts = nullptr, runtime::catalog* catalog = nullptr);

        /**
         * Gets the catalog being compiled.
         * @return Returns the catalog being compiled or nullptr if catalog expressions are not supported.
         */
        runtime::catalog* catalog();

        /**
         * Gets the current scope.
         * @return Returns the current scope and will never return nullptr.
         */
        std::shared_ptr<runtime::scope> const& current_scope();

        /**
         * Gets the top scope.
         * @return Returns the top scope and will never return nullptr.
         */
        std::shared_ptr<runtime::scope> const& top_scope();

        /**
         * Gets the node scope.
         * @return Returns the node scope or nullptr if there currently is no node scope.
         */
        std::shared_ptr<runtime::scope> const& node_scope();

        /**
         * Gets the node or top scope.
         * @return Returns the node scope if there is one, otherwise returns the top scope.
         */
        std::shared_ptr<runtime::scope> const& node_or_top();

        /**
         * Adds a scope to the evaluation context.
         * @param scope The scope to add to the evaluation context.
         * @return Returns true if the scope was added or false if the scope already exists.
         */
        bool add_scope(std::shared_ptr<runtime::scope> scope);

        /**
         * Finds a scope by name.
         * @param name The name of the scope to find.
         * @return Returns a pointer to the scope if found or nullptr if the scope is not found.
         */
        std::shared_ptr<runtime::scope> find_scope(std::string const& name) const;

        /**
         * Sets the given matches into the context.
         * Note: This member function has no effect unless a match scope is present.
         * @param matches The matches to set.
         */
        void set(std::smatch const& matches);

        /**
         * Looks up a variable's value.
         * @param name The name of the variable to look up.
         * @param evaluator The expression evaluator to use to log warnings for scope lookup failures.  Requires a position.
         * @param position The position where the lookup is taking place or nullptr if not in source.
         * @return Returns the variable's value or nullptr if the variable was not found.
         */
        std::shared_ptr<values::value const> lookup(std::string const& name, expression_evaluator* evaluator = nullptr, lexer::position const* position = nullptr);

        /**
         * Looks up a match variable value by index.
         * @param index The index of the match variable.
         * @return Returns the match variable's value or nullptr if the variable wasn't found.
         */
        std::shared_ptr<values::value const> lookup(size_t index) const;

        /**
         * Creates a match scope.
         * @return Returns the match scope.
         */
        match_scope create_match_scope();

        /**
         * Creates a local scope.
         * @param scope The parent scope to inherit from; if null, the current scope will be used.
         * @return Returns the local scope.
         */
        local_scope create_local_scope(std::shared_ptr<runtime::scope> scope = nullptr);

     private:
        friend struct match_scope;
        friend struct local_scope;
        friend struct node_scope;

        runtime::catalog* _catalog;
        std::unordered_map<std::string, std::shared_ptr<runtime::scope>> _scopes;
        std::vector<std::shared_ptr<runtime::scope>> _scope_stack;
        std::shared_ptr<runtime::scope> _node_scope;
        std::vector<std::shared_ptr<std::vector<std::shared_ptr<values::value const>>>> _match_stack;
    };

}}  // namespace puppet::runtime
