/**
 * @file
 * Declares the Puppet catalog.
 */
#pragma once

#include "../lexer/position.hpp"
#include "../compiler/context.hpp"
#include "values/value.hpp"
#include <string>
#include <functional>
#include <unordered_set>
#include <unordered_map>

namespace puppet { namespace runtime {

    // Forward declaration for catalog.
    struct catalog;

    // Forward declaration for context.
    struct context;

    // Forward declaration of scope.
    struct scope;

    // Forward declaration of resource.
    struct resource;

    /**
     * Represents a class definition in a catalog.
     */
    struct class_definition
    {
        /**
         * Constructs a class definition.
         * @param catalog The catalog containing this class definition.
         * @param klass The qualified class name for the class.
         * @param context The compilation context for the class.
         * @param expression The class definition expression.
         */
        class_definition(runtime::catalog& catalog, types::klass klass, std::shared_ptr<compiler::context> context, ast::class_definition_expression const* expression);

        /**
         * Gets the catalog containing this class definition.
         * @return Returns the catalog containing this class definition.
         */
        runtime::catalog const& catalog() const;

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
        std::shared_ptr<runtime::scope> evaluate_parent(runtime::context& context);

        runtime::catalog& _catalog;
        types::klass _klass;
        types::klass _parent;
        std::shared_ptr<compiler::context> _context;
        ast::class_definition_expression const* _expression;
        std::shared_ptr<std::string> _path;
        size_t _line;
    };

    /**
     * Represents a defined type in a catalog.
     */
    struct defined_type
    {
        /**
         * Constructs a defined type.
         * @param catalog The catalog containing the defined type.
         * @param type The resource type for the defined type.
         * @param context The compilation context for the defined type.
         * @param expression The defined type expression.
         */
        defined_type(runtime::catalog& catalog, std::string type, std::shared_ptr<compiler::context> context, ast::defined_type_expression const& expression);

        /**
         * Gets the catalog containing this defined type.
         * @return Returns the catalog containing this defined type.
         */
        runtime::catalog const& catalog() const;

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
        runtime::catalog& _catalog;
        std::string _type;
        std::shared_ptr<compiler::context> _context;
        ast::defined_type_expression const& _expression;
    };

    /**
     * Represents a node definition in a catalog.
     */
    struct node_definition
    {
        /**
         * Constructs a node definition.
         * @param catalog The catalog containing the node definition.
         * @param context The compilation context for the node definition.
         * @param expression The node definition expression.
         */
        node_definition(runtime::catalog& catalog, std::shared_ptr<compiler::context> context, ast::node_definition_expression const& expression);

        /**
         * Gets the catalog containing this node definition.
         * @return Returns the catalog containing this node definition.
         */
        runtime::catalog const& catalog() const;

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
        runtime::catalog& _catalog;
        std::shared_ptr<compiler::context> _context;
        ast::node_definition_expression const& _expression;
    };

    /**
     * Represents a declared resource in a catalog.
     */
    struct resource
    {
        /**
         * Creates a resource with the given type and title.
         * @param catalog The catalog that contains the resource.
         * @param type The resource type (e.g. File['/tmp/foo']).
         * @param path The path of the file defining the resource.
         * @param line The line defining the resource.
         * @param exported True if the resource is exported or false if not.
         */
        resource(runtime::catalog& catalog, types::resource type, std::shared_ptr<std::string> path, size_t line, bool exported = false);

        /**
         * Gets the catalog that contains the resource.
         * @return Returns the catalog that contains the resource.
         */
        runtime::catalog const& catalog() const;

        /**
         * Gets the resource type of the resource.
         * @return Returns the resource type of the resource.
         */
        types::resource const& type() const;

        /**
         * Gets the path of the file where the resource was defined.
         * @return Returns the path of the file where the resource was defined.
         */
        std::string const& path() const;

        /**
         * Gets the line where the resource was defined.
         * @return Returns the line where the resource was defined.
         */
        size_t line() const;

        /**
         * Gets the tags of the resource.
         * @return Returns the tags of the resource.
         */
        std::unordered_set<std::string> const& tags() const;

        /**
         * Gets the parameters of the resource.
         * @return Returns the parameters of the resource.
         */
        std::unordered_map<std::string, values::value> const& parameters() const;

        /**
         * Gets whether or not the resource is exported.
         * @return Returns true if the resource is exported or false if it is not.
         */
        bool exported() const;

        /**
         * Adds a tag to the resource.
         * @param tag The tag to add to the resource.
         */
        void add_tag(std::string tag);

        /**
         * Sets a parameter's value.
         * @param name The parameter name.
         * @param name_position The position of the parameter's name in the input.
         * @param value The parameter's value.
         * @param value_position The position of the parameter's value in the input.
         * @param override True if the parameter's value should be overridden or false if not.
         */
        void set_parameter(std::string const& name, lexer::position const& name_position, values::value value, lexer::position const& value_position, bool override = false);

        /**
         * Removes a parameter from the resource.
         * @param name The name of the parameter to remove.
         * @return Returns true if the parameter was removed or false if it did not exist.
         */
        bool remove_parameter(std::string const& name);

     private:
        void store_parameter(std::string const& name, lexer::position const& name_position, values::value value, bool override);
        bool handle_metaparameter(std::string const& name, lexer::position const& name_position, values::value& value, lexer::position const& value_position);
        void create_alias(values::value const& value, lexer::position const& position);

        runtime::catalog& _catalog;
        types::resource _type;
        std::shared_ptr<std::string> _path;
        size_t _line;
        std::unordered_set<std::string> _tags;
        std::unordered_map<std::string, values::value> _parameters;
        bool _exported;
    };

    /**
     * Represents the Puppet catalog.
     */
    struct catalog
    {
        /**
         * Constructs a catalog.
         */
        catalog();

        /**
         * Finds a resource in the catalog.
         * @param resource The resource type to find.
         * @return Returns the resource if found or nullptr if the resource is not in the catalog.
         */
        runtime::resource* find_resource(types::resource const& resource);

        /**
         * Aliases a resource.
         * @param resource The reource to alias.
         * @param alias The new alias name.
         * @return Returns true if the resource was aliased or false if a resource with that name or alias already exists.
         */
        bool alias_resource(types::resource const& resource, std::string const& alias);

        /**
         * Adds a resource to the catalog.
         * @param resource The qualified resource to add.
         * @param path The path of the file defining the resource.
         * @param line The line defining the resource.
         * @param exported True if the resource is exported or false if it is not.
         * @return Returns the new resource in the catalog or nullptr if the resource already exists in the catalog.
         */
        runtime::resource* add_resource(types::resource resource, std::shared_ptr<std::string> path, size_t line, bool exported = false);

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
         * @param context The evaluation context to use.
         * @param klass The class to declare.
         * @param path The path to the file that is declaring the resource.
         * @param position The position where the resource is declared.
         * @param arguments The class arguments or nullptr for no arguments.
         * @return Returns the resource that was added for the class or nullptr if the class failed to evaluate.
         */
        runtime::resource* declare_class(runtime::context& context, types::klass const& klass, std::shared_ptr<std::string> path, lexer::position const& position, std::unordered_map<ast::name, values::value> const* arguments = nullptr);

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
         * @param context The evaluation context to use.
         * @param type The defined type name.
         * @param title The resource title.
         * @param path The path to the file that is declaring the resource.
         * @param position The position where the resource is declared.
         * @param arguments The defined type's arguments or nullptr for no arguments.
         * @return Returns the resource that was added to the catalog or nullptr if the defined type failed to evaluate.
         */
        runtime::resource* declare_defined_type(runtime::context& context, std::string const& type, std::string const& title, std::shared_ptr<std::string> path, lexer::position const& position, std::unordered_map<ast::name, values::value> const* arguments = nullptr);

        /**
         * Defines a node.
         * @param context The compilation context.
         * @param expression The node definition expression.
         */
        void define_node(std::shared_ptr<compiler::context> context, ast::node_definition_expression const& expression);

        /**
         * Evaluates a node definition for the given node.
         * @param context The evaluation context to use.
         * @param node The node to evaluate for.
         * @return Returns true if evaluation was successful or false if it was not.
         */
        bool evaluate_node(runtime::context& context, compiler::node const& node);

     private:
        void validate_parameters(bool klass, std::vector<ast::parameter> const& parameters);

        std::unordered_map<std::string, std::unordered_map<std::string, resource>> _resources;
        std::unordered_map<std::string, std::unordered_map<std::string, resource*>> _aliases;
        std::unordered_map<types::klass, std::vector<class_definition>, boost::hash<types::klass>> _classes;
        std::unordered_map<std::string, defined_type> _defined_types;
        std::vector<node_definition> _nodes;
        std::unordered_map<std::string, size_t> _named_nodes;
        std::vector<std::pair<values::regex, size_t>> _regex_node_definitions;
        ssize_t _default_node_index;
    };

}}  // puppet::runtime
