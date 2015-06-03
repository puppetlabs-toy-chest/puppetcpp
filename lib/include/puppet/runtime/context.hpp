/**
 * @file
 * Declares the Puppet evaluation context.
 */
#pragma once

#include "../lexer/position.hpp"
#include "../ast/syntax_tree.hpp"
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
     * Represents a class definition used in evaluation.
     */
    struct class_definition
    {
        /**
         * Constructs a class definition.
         * @param tree The syntax tree containing the class definition expression.
         * @param expression The class definition expression.
         */
        class_definition(std::shared_ptr<ast::syntax_tree> tree, ast::class_definition_expression const* expression);

        /**
         * Determines if the class has been evaluated.
         * @return Returns true if the class has been evaluated or false if it has not.
         */
        bool evaluated() const;

        /**
         * Gets the syntax tree containing the class definition expression.
         * @return Returns the syntax tree containing the class definition expression or nullptr if the class was evaluated.
         */
        ast::syntax_tree const* tree() const;

        /**
         * Gets the class definition expression.
         * @return Returns the class definition expression or nullptr if the class was evaluated.
         */
        ast::class_definition_expression const* expression() const;

        /**
         * Gets the path of the file containing the class definition.
         * @return Returns the path of the file containing the class definition.
         */
        std::string const& path() const;

        /**
         * Gets the line number of the class definition.
         * @return Returns the line number of the class definition.
         */
        size_t line() const;

        /**
         * Releases resources after the definition has been evaluated.
         */
        void release();

     private:
        std::shared_ptr<ast::syntax_tree> _tree;
        ast::class_definition_expression const* _expression;
        std::string _path;
        size_t _line;
    };

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
         * Adds a scope to the evaluation context.
         * Note: if a scope of the same name already exists, the existing scope is returned unmodified.
         * @param scope The scope to add.
         * @return Returns the scope that was added.
         */
        runtime::scope* add_scope(runtime::scope scope);

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
         * Looks up a variable.
         * @param name The name of the variable to look up.
         * @param position The position where the lookup is taking place or nullptr if not in source.
         * @return Returns a pointer to the variable if found or nullptr if the variable was not found.
         */
        values::value const* lookup(std::string const& name, lexer::position const* position = nullptr);

        /**
         * Emits a warning with the given position and message.
         * @param position The position of the warning.
         * @param message The warning message.
         */
        void warn(lexer::position const& position, std::string const& message) const;

        /**
         * Defines a class in the evaluation context.
         * @param tree The abstract syntax tree containing the class definition.
         * @param expression The class definition expression.
         * @return Returns the new class definition or nullptr if the class already exists in the catalog.
         */
        class_definition* define_class(std::shared_ptr<ast::syntax_tree> tree, ast::class_definition_expression const& expression);

        /**
         * Finds a class definition.
         * @param name The name of the class to find.
         * @return Returns the class definition or nullptr if the class does not exist.
         */
        class_definition* find_class(std::string const& name);

        /**
         * Finds a class definition.
         * @param name The name of the class to find.
         * @return Returns the class definition or nullptr if the class does not exist.
         */
        class_definition const* find_class(std::string const& name) const;

     private:
        logging::logger& _logger;
        compiler::node& _node;
        runtime::catalog& _catalog;
        std::unordered_map<std::string, runtime::scope> _scopes;
        std::deque<runtime::scope*> _scope_stack;
        std::unordered_map<std::string, class_definition> _classes;
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
