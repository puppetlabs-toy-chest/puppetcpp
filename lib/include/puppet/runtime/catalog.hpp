/**
 * @file
 * Declares the Puppet catalog.
 */
#pragma once

#include "../lexer/position.hpp"
#include "../compiler/context.hpp"
#include "values/value.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace puppet { namespace runtime {

    // Forward declaration for catalog.
    struct catalog;

    // Forward declaration for context.
    struct context;

    // Forward declaration of scope.
    struct scope;

    // Forward declaration of evaluation_exception.
    struct evaluation_exception;

    /**
     * Represents a collection of resource attributes.
     */
    struct attributes
    {
        /**
         * Constructs an attribute collection.
         * @param parent The parent attributes to inherit values from.
         */
        explicit attributes(std::shared_ptr<attributes const> parent = nullptr);

        /**
         * Gets an attribute.
         * @param name The name of the attribute to get.
         * @param check_parent True if the parent should be checked for the attribute or false if not.
         * @return Returns the value if the attribute exists or nullptr if the attribute does not exist.
         */
        std::shared_ptr<values::value const> get(std::string const& name, bool check_parent = true) const;

        /**
         * Sets an attribute.
         * @param name The name of the attribute to set.
         * @param value The value to set.
         */
        void set(std::string const& name, values::value value);

        /**
         * Appends a value to an existing attribute.
         * If the attribute does not exist, the attribute is set to the value as an array.
         * @param name The name of the attribute to append to.
         * @param value The value to append.
         * @param append_duplicates True if duplicate values in the resulting array are appended or false if they are not.
         * @return Returns true if the value was appended or false if the attribute already exists and is not an array.
         */
        bool append(std::string const& name, values::value value, bool append_duplicates = true);

        /**
         * Enumerates each stored attribute.
         * @param callback The callback to call for each stored attribute.
         */
        void each(std::function<bool(std::string const& name, std::shared_ptr<values::value const> const& value)> const& callback) const;

     private:
        std::shared_ptr<attributes const> _parent;
        std::unordered_map<std::string, std::shared_ptr<values::value>> _values;
    };

    /**
     * Represents the possible resource relationship types.
     */
    enum class relationship
    {
        /**
         * Class or defined type containment.
         */
        contains,
        /**
         * The "before" metaparam on source or -> operator.
         */
        before,
        /**
         * The "require" metaparam on target or <- operator.
         */
        require,
        /**
         * The "notify" metaparam on source or ~> operator.
         */
        notify,
        /**
         * The "subscribe" metaparam on target or <~ operator.
         */
        subscribe
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
         * @param attributes The resource's attributes.
         * @param exported True if the resource is exported or false if not.
         */
        resource(
            runtime::catalog& catalog,
            types::resource type,
            std::shared_ptr<std::string> path = nullptr,
            size_t line = 0,
            std::shared_ptr<attributes> attributes = nullptr,
            bool exported = false);

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
         * Gets the path of the file where the resource was declared.
         * @return Returns the path of the file where the resource was declared.
         */
        std::shared_ptr<std::string> const& path() const;

        /**
         * Gets the line where the resource was declared.
         * @return Returns the line where the resource was declared or 0 if not declared in a source file.
         */
        size_t line() const;

        /**
         * Gets whether or not the resource is exported.
         * @return Returns true if the resource is exported or false if it is not.
         */
        bool exported() const;

        /**
         * Gets the resource's attributes.
         * @return Returns the resource's attribute.
         */
        runtime::attributes& attributes();

        /**
         * Gets the resource's attributes.
         * @return Returns the resource's attribute.
         */
        runtime::attributes const& attributes() const;

        /**
         * Makes the resource's attributes unique.
         */
        void make_attributes_unique();

        /**
         * Determines if the given name is a metaparameter name.
         * @param name The name to check.
         * @return Returns true if name is the name of a metaparameter or false if not.
         */
        static bool is_metaparameter(std::string const& name);

     private:
        friend struct catalog;

        size_t vertex_id() const;
        void vertex_id(size_t id);

        runtime::catalog& _catalog;
        types::resource _type;
        std::shared_ptr<std::string> _path;
        size_t _line;
        std::shared_ptr<runtime::attributes> _attributes;
        size_t _vertex_id;
        bool _exported;
    };

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
         * @param resource The resource representing the class.
         * @param create_exception The callback to use to create an exception for the given argument name and message.
         * @return Returns true if the evaluation was successful or false if the evaluation failed.
         */
        bool evaluate(
            runtime::context& context,
            runtime::resource const& resource,
            std::function<evaluation_exception(bool, std::string const&, std::string)> const& create_exception);

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
         * @param create_exception The callback to use to create an exception for the given argument name and message.
         * @return Returns true if the evaluation was successful or false if the evaluation failed.
         */
        bool evaluate(
            runtime::context& context,
            runtime::resource const& resource,
            std::function<evaluation_exception(bool, std::string const&, std::string)> const& create_exception);

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
     * Represnce a resource dependency graph.
     */
    using dependency_graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, resource*, relationship>;

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
         * Gets the catalog's dependency graph.
         * The dependency graph is only populated after a call to catalog::finalize().
         * @return Returns the catalog's dependency graph.
         */
        dependency_graph const& graph() const;

        /**
         * Finds a resource in the catalog.
         * @param resource The resource type to find.
         * @return Returns the resource if found or nullptr if the resource is not in the catalog.
         */
        runtime::resource* find_resource(types::resource const& resource);

        /**
         * Adds a resource to the catalog.
         * @param type The qualified resource type to add.
         * @param path The path of the file defining the resource.
         * @param line The line defining the resource.
         * @param attributes The resource's initial attributes.
         * @param exported True if the resource is exported or false if it is not.
         * @return Returns the new resource in the catalog or nullptr if the resource already exists in the catalog.
         */
        runtime::resource* add_resource(
            types::resource type,
            std::shared_ptr<std::string> path,
            size_t line,
            std::shared_ptr<attributes> attributes = nullptr,
            bool exported = false);

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
         * @param path The path to the file that is declaring the resource or nullptr if not declared in a source file.
         * @param line The line in the file where the class is being declared or 0 if not declared in a source file.
         * @param attributes The class resource's attributes or nullptr for an empty set.
         * @param create_exception The callback to use to create an exception for the given argument name and message.
         * @return Returns the resource that was added for the class or nullptr if the class failed to evaluate.
         */
        runtime::resource* declare_class(
            runtime::context& context,
            types::klass const& klass,
            std::shared_ptr<std::string> path = nullptr,
            size_t line = 0,
            std::shared_ptr<runtime::attributes> attributes = nullptr,
            std::function<evaluation_exception(bool, std::string const&, std::string)> const& create_exception = nullptr);

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
         * @param path The path to the file that is declaring the defined type or nullptr if not declared in a source file.
         * @param line The line in the file where the defined type is being declared or 0 if not declared in a source file.
         * @param attributes The defined type resource's attributes or nullptr for an empty set.
         * @param create_exception The callback to use to create an exception for the given argument name and message.
         * @return Returns the resource that was added to the catalog or nullptr if the defined type failed to evaluate.
         */
        runtime::resource* declare_defined_type(
            runtime::context& context,
            std::string const& type,
            std::string const& title,
            std::shared_ptr<std::string> path = nullptr,
            size_t line = 0,
            std::shared_ptr<runtime::attributes> attributes = nullptr,
            std::function<evaluation_exception(bool, std::string const&, std::string)> const& create_exception = nullptr);

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

        /**
         * Finalizes the catalog.
         * Populates the dependency graph and any needed default resources.
         */
        void finalize();

     private:
        void validate_parameters(bool klass, std::shared_ptr<compiler::context> const& context, std::vector<ast::parameter> const& parameters);
        void populate_graph();
        void process_relationship_parameter(resource const& source, std::string const& name, runtime::relationship relationship);
        void add_relationship(runtime::relationship relationship, runtime::resource const& source, runtime::resource const& target);
        void detect_cycles();

        std::unordered_map<std::string, std::unordered_map<std::string, resource>> _resources;
        std::unordered_map<types::klass, std::vector<class_definition>, boost::hash<types::klass>> _classes;
        std::unordered_map<std::string, defined_type> _defined_types;
        std::vector<node_definition> _nodes;
        std::unordered_map<std::string, size_t> _named_nodes;
        std::vector<std::pair<values::regex, size_t>> _regex_node_definitions;
        ssize_t _default_node_index;
        dependency_graph _graph;
    };

}}  // puppet::runtime
