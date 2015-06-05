/**
 * @file
 * Declares the Puppet evaluation context.
 */
#pragma once

#include "../lexer/position.hpp"
#include "../compiler/context.hpp"
#include "../runtime/values/value.hpp"
#include "../logging/logger.hpp"
#include "scope.hpp"
#include "catalog.hpp"
#include <string>
#include <memory>
#include <deque>
#include <unordered_map>
#include <functional>

namespace puppet { namespace runtime {

    // Forward declaration of context.
    struct context;

    /**
     * Represents a class definition used in evaluation.
     */
    struct class_definition
    {
        /**
         * Constructs a class definition.
         * @param klass The qualified class name for the class.
         * @param context The compilation context for the class.
         * @param expression The class definition expression.
         */
        class_definition(types::klass klass, std::shared_ptr<compiler::context> context, ast::class_definition_expression const* expression);

        /**
         * Gets the qualified class type for this class definition.
         * @return Returns the qualified class type for this class definition.
         */
        types::klass const& klass() const;

        /**
         * Gets the parent class or nullptr if there is no parent class.
         * @return Returns the parent class or nullptr if there is no parent class.
         */
        types::klass const* parent() const;

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
         * Determines if the class has been evaluated.
         * @return Returns true if the class has been evaluated or false if it has not.
         */
        bool evaluated() const;

        /**
         * Evaluates the class.
         * @param context The evaluation context.
         * @param arguments The arguments to the class.
         * @return Returns true if the evaluation was successful or false if the evaluation failed.
         */
        bool evaluate(runtime::context& context, std::unordered_map<ast::name, values::value> const* arguments = nullptr);

     private:
        runtime::scope* evaluate_parent(runtime::context& context);

        types::klass _klass;
        types::klass _parent;
        std::shared_ptr<compiler::context> _context;
        ast::class_definition_expression const* _expression;
        std::shared_ptr<std::string> _path;
        size_t _line;
    };

    /**
     * Represents the evaluation context.
     */
    struct context
    {
        /**
         * Constructs an evaluation context.
         * @param catalog The catalog being compiled.
         */
        explicit context(runtime::catalog& catalog);

        /**
         * Gets the catalog being compiled.
         * @return Returns the catalog being compiled.
         */
        runtime::catalog& catalog();

        /**
         * Gets the current scope.
         * @return Returns the current scope.
         */
        runtime::scope& scope();

        /**
         * Gets the top scope.
         * @return Returns the top scope.
         */
        runtime::scope& top();

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

        /**
         * Defines a class in the evaluation context.
         * @param klass The class to define.
         * @param context The compilation context.
         * @param expression The class definition expression.
         * @return Returns nullptr if the class was successfully defined or existing class definition that cannot be merged with the given definition.
         */
        class_definition const* define_class(types::klass klass, std::shared_ptr<compiler::context> context, ast::class_definition_expression const& expression);

        /**
         * Declares a class.
         * If the class is already declared, the existing class will be returned.
         * @param klass The class to declare.
         * @param path The path to the file that is declaring the resource.
         * @param position The position where the resource is declared.
         * @param arguments The class arguments or nullptr for no arguments.
         * @return Returns the resource that was added for the class or nullptr if the class failed to evaluate.
         */
        runtime::resource* declare_class(types::klass const& klass, std::shared_ptr<std::string> path, lexer::position const& position, std::unordered_map<ast::name, values::value> const* arguments = nullptr);

        /**
         * Determines if a class is defined.
         * @param klass The class to check.
         * @return Returns true if the class is defined or false if not.
         */
        bool is_class_defined(types::klass const& klass) const;

        /**
         * Determines if a class is declared.
         * @param klass The class to check.
         * @return Returns true if the class is declared or false if not.
         */
        bool is_class_declared(types::klass const& klass) const;

     private:
        void validate_parameters(bool klass, std::vector<ast::parameter> const& parameters);

        runtime::catalog& _catalog;
        std::unordered_map<std::string, runtime::scope> _scopes;
        std::deque<runtime::scope*> _scope_stack;
        std::unordered_map<types::klass, std::vector<class_definition>, boost::hash<types::klass>> _classes;
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

}}  // namespace puppet::runtime
