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
     * Represents a defined type.
     */
    struct defined_type
    {
        /**
         * Constructs a defined type.
         * @param type The resource type for the defined type.
         * @param context The compilation context for the defined type.
         * @param expression The defined type expression.
         */
        defined_type(std::string type, std::shared_ptr<compiler::context> context, ast::defined_type_expression const& expression);

        /**
         * Gets the resource type of the defined type.
         * @return Returns the resource type of the defined type.
         */
        std::string const& type() const;

        /**
         * Gets the path of the file containing the defined type.
         * @return Returns the path of the file containing the defined type.
         */
        std::string const& path() const;

        /**
         * Gets the line number of the defined type.
         * @return Returns the line number of the defined type.
         */
        size_t line() const;

        /**
         * Evaluates the defined type.
         * @param context The evaluation context.
         * @param resource The resource for the defined type.
         * @param arguments The arguments to the defined type.
         * @return Returns true if the evaluation was successful or false if the evaluation failed.
         */
        bool evaluate(runtime::context& context, runtime::resource const& resource, std::unordered_map<ast::name, values::value> const* arguments = nullptr);

     private:
        std::string _type;
        std::shared_ptr<compiler::context> _context;
        ast::defined_type_expression const& _expression;
    };

    /**
     * Represents a node definition.
     */
    struct node_definition
    {
        /**
         * Constructs a node definition.
         * @param context The compilation context for the node definition.
         * @param expression The node definition expression.
         */
        node_definition(std::shared_ptr<compiler::context> context, ast::node_definition_expression const& expression);

        /**
         * Gets the path of the file containing the node definition.
         * @return Returns the path of the file containing the node definition.
         */
        std::string const& path() const;

        /**
         * Gets the line number of the node definition.
         * @return Returns the line number of the node definition.
         */
        size_t line() const;

        /**
         * Evaluates the node definition.
         * @param context The evaluation context.
         * @return Returns true if the evaluation succeeded or false if it did not.
         */
        bool evaluate(runtime::context& context);

     private:
        std::shared_ptr<compiler::context> _context;
        ast::node_definition_expression const& _expression;
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

        /**
         * Defines a class in the evaluation context.
         * @param klass The class to define.
         * @param context The compilation context.
         * @param expression The class definition expression.
         */
        void define_class(types::klass klass, std::shared_ptr<compiler::context> context, ast::class_definition_expression const& expression);

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

        /**
         * Defines a (defined) type in the evaluation context.
         * @param type The type to define.
         * @param context The compilation context.
         * @param expression The defined type expression.
         */
        void define_type(std::string type, std::shared_ptr<compiler::context> context, ast::defined_type_expression const& expression);

        /**
         * Determines if the given type name is a defined type.
         * @param type The type name to check.
         * @return Returns true if the type is a defined type or false if it is not.
         */
        bool is_defined_type(std::string const& type) const;

        /**
         * Declares a defined type.
         * @param type The defined type name.
         * @param title The resource title.
         * @param path The path to the file that is declaring the resource.
         * @param position The position where the resource is declared.
         * @param arguments The defined type's arguments or nullptr for no arguments.
         * @return Returns the resource that was added to the catalog or nullptr if the defined type failed to evaluate.
         */
        runtime::resource* declare_defined_type(std::string const& type, std::string const& title, std::shared_ptr<std::string> path, lexer::position const& position, std::unordered_map<ast::name, values::value> const* arguments = nullptr);

        /**
         * Defines a node.
         * @param context The compilation context.
         * @param expression The node definition expression.
         */
        void define_node(std::shared_ptr<compiler::context> context, ast::node_definition_expression const& expression);

        /**
         * Evaluates a node definition for the given node.
         * @param node The node to evaluate for.
         * @return Returns true if evaluation was successful or false if it was not.
         */
        bool evaluate_node(compiler::node const& node);

     private:
        void validate_parameters(bool klass, std::vector<ast::parameter> const& parameters);
        friend struct node_scope_helper;

        runtime::catalog& _catalog;
        std::unordered_map<std::string, runtime::scope> _scopes;
        std::deque<runtime::scope*> _scope_stack;
        std::unique_ptr<runtime::scope> _node_scope;
        std::unordered_map<types::klass, std::vector<class_definition>, boost::hash<types::klass>> _classes;
        std::unordered_map<std::string, defined_type> _defined_types;
        std::vector<node_definition> _nodes;
        std::unordered_map<std::string, size_t> _named_nodes;
        std::vector<std::pair<values::regex, size_t>> _regex_node_definitions;
        ssize_t _default_node_index;
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
