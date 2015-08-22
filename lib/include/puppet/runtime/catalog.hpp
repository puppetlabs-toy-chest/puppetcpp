/**
 * @file
 * Declares the Puppet catalog.
 */
#pragma once

#include "../lexer/position.hpp"
#include "../ast/syntax_tree.hpp"
#include "values/value.hpp"
#include <boost/optional.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <exception>
#include <ostream>

namespace puppet { namespace compiler {

    // Forward declaration of compiler context.
    struct context;

    // Forward declaration of node.
    struct node;

}}  // namespace puppet::compiler

namespace puppet { namespace runtime {

    // Forward declaration for catalog.
    struct catalog;

    // Forward declaration for context.
    struct context;

    // Forward declaration of scope.
    struct scope;

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
         * Resource containment.
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
     * Stream insertion operator for relationship.
     * @param out The output stream to write the relationship to.
     * @param relationship The relationship to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& out, runtime::relationship relationship);

    /**
     * Represents a declared resource in a catalog.
     */
    struct resource
    {
        /**
         * Creates a resource with the given type and title.
         * @param type The resource type (e.g. File['/tmp/foo']).
         * @param path The path of the file defining the resource.
         * @param line The line defining the resource.
         * @param attributes The resource's attributes.
         * @param exported True if the resource is exported or false if not.
         */
        resource(
            types::resource type,
            std::shared_ptr<std::string> path = nullptr,
            size_t line = 0,
            std::shared_ptr<attributes> attributes = nullptr,
            bool exported = false);

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
         * Gets the vertex id of the resource in the catalog dependency graph.
         * @return Returns the vertex id of the resource in the catalog dependency graph.
         */
        size_t vertex_id() const;

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
        void vertex_id(size_t id);

        types::resource _type;
        std::shared_ptr<std::string> _path;
        size_t _line;
        std::shared_ptr<runtime::attributes> _attributes;
        size_t _vertex_id;
        bool _exported;
    };

    /**
     * Represents a class definition.
     */
    struct class_definition
    {
        /**
         * Constructs a class definition.
         * @param klass The qualified class name for the class.
         * @param context The compilation context for the class.
         * @param expression The class definition expression.
         */
        class_definition(types::klass klass, std::shared_ptr<compiler::context> context, ast::class_definition_expression const& expression);

        /**
         * Gets the qualified class type for this class definition.
         * @return Returns the qualified class type for this class definition.
         */
        types::klass const& klass() const;

        /**
         * Gets the optional parent class.
         * @return Returns the optional parent class.
         */
        boost::optional<types::klass> const& parent() const;

        /**
         * Gets the path of the file containing the class definition.
         * @return Returns the path of the file containing the class definition or nullptr if not defined in a file.
         */
        std::shared_ptr<std::string> const& path() const;

        /**
         * Gets the line number of the class definition.
         * @return Returns the line number of the class definition.
         */
        size_t line() const;

     private:
        friend struct catalog;
        void evaluate(runtime::context& context, runtime::resource& resource);

        std::shared_ptr<runtime::scope> evaluate_parent(runtime::context& context);

        types::klass _klass;
        boost::optional<types::klass> _parent;
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
        std::shared_ptr<std::string> const& path() const;

        /**
         * Gets the line number of the defined type.
         * @return Returns the line number of the defined type.
         */
        size_t line() const;

     private:
        friend struct catalog;
        void evaluate(runtime::context& context, runtime::resource& resource);

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
         * Gets the compilation context for the node definition.
         * @return Returns the compilation context for the node definition.
         */
        std::shared_ptr<compiler::context> const& context() const;

        /**
         * Gets the position of the node definition.
         * @return Returns the position of the node definition.
         */
        lexer::position const& position() const;

     private:
        friend struct catalog;
        void evaluate(runtime::context& context);

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
         * Gets the catalog's dependency graph.
         * The dependency graph is only populated after a call to catalog::finalize().
         * @return Returns the catalog's dependency graph.
         */
        dependency_graph const& graph() const;

        /**
         * Adds a relationship (i.e. an edge) to the dependency graph.
         * The source will become dependent upon the target (reversed for before and notify relationships).
         * @param relationship The relationship from the source to the target.
         * @param source The source resource.
         * @param target The target resource.
         */
        void add_relationship(runtime::relationship relationship, runtime::resource const& source, runtime::resource const& target);

        /**
         * Finds a resource in the catalog.
         * @param resource The resource type to find.
         * @return Returns the resource if found or nullptr if the resource is not in the catalog.
         */
        runtime::resource* find_resource(types::resource const& resource);

        /**
         * Adds a resource to the catalog.
         * @param evaluation_context The current evaluation context.
         * @param type The qualified resource type to add.
         * @param compilation_context The compilation context where the resource is being declared.
         * @param position The position where the resource is being declared.
         * @param attributes The resource's initial attributes.
         * @param exported True if the resource is exported or false if it is not.
         * @return Returns the resource that was added.
         */
        runtime::resource& add_resource(
            runtime::context& evaluation_context,
            types::resource type,
            std::shared_ptr<compiler::context> const& compilation_context,
            lexer::position const& position,
            std::shared_ptr<runtime::attributes> attributes = nullptr,
            bool exported = false);

        /**
         * Finds the definitions of a class.
         * @param klass The class to find.
         * @return Returns the class definitions if defined or nullptr if not defined.
         */
        std::vector<class_definition> const* find_class(types::klass const& klass);

        /**
         * Defines a class.
         * Multiple class definitions may exist for the same class.
         * @param klass The class to define.
         * @param context The compilation context where the class is defined.
         * @param expression The class definition expression.
         */
        void define_class(
            types::klass klass,
            std::shared_ptr<compiler::context> const& context,
            ast::class_definition_expression const& expression);

        /**
         * Declares a class.
         * If the class is already declared, the existing class resource will be returned.
         * @param evaluation_context The current evaluation context.
         * @param type The resource type for the class.
         * @param compilation_context The compilation context where the class is being declared.
         * @param position The position where the class is being declared.
         * @param attributes The class resource's attributes or nullptr for an empty set.
         * @return Returns the class resource that was added to the catalog.
         */
        runtime::resource& declare_class(
            runtime::context& evaluation_context,
            types::resource type,
            std::shared_ptr<compiler::context> const& compilation_context,
            lexer::position const& position,
            std::shared_ptr<runtime::attributes> attributes = nullptr);

        /**
         * Finds a defined type's definition.
         * @param type The type name of the defined type.
         * @return Returns the defined type's definition if defined or nullptr if not defined.
         */
        defined_type const* find_defined_type(std::string const& type);

        /**
         * Defines a defined type.
         * Only one definition of a defined type may exist.
         * @param type The name of the defined type.
         * @param context The compilation context where the defined type is defined.
         * @param expression The defined type expression.
         */
        void define_type(
            std::string type,
            std::shared_ptr<compiler::context> const& context,
            ast::defined_type_expression const& expression);

        /**
         * Declares a defined type.
         * A defined type cannot be declared more than once.
         * @param evaluation_context The current evaluation context.
         * @param type_name The lower-case type name for the defined type.
         * @param type The resource type for the defined type.
         * @param compilation_context The compilation context where the class is being declared.
         * @param position The position where the class is being declared.
         * @param attributes The defined type resource's attributes or nullptr for an empty set.
         * @return Returns the defined type resource that was added to the catalog.
         */
        runtime::resource& declare_defined_type(
            runtime::context& evaluation_context,
            std::string const& type_name,
            types::resource type,
            std::shared_ptr<compiler::context> const& compilation_context,
            lexer::position const& position,
            std::shared_ptr<runtime::attributes> attributes = nullptr);

        /**
         * Defines a node.
         * @param context The compilation context where the node is defined.
         * @param expression The node definition expression.
         */
        void define_node(std::shared_ptr<compiler::context> const& context, ast::node_definition_expression const& expression);

        /**
         * Declares a node.
         * @param evaluation_context The current evaluation context.
         * @param node The node to declare.
         * @return Returns the node resource that was added to the catalog or nullptr if there are no node definitions.
         */
        runtime::resource* declare_node(runtime::context& evaluation_context, compiler::node const& node);

        /**
         * Finalizes the catalog.
         * Populates the dependency graph and adds any needed resources.
         */
        void finalize();

        /**
         * Writes the dependency graph as a DOT file.
         * @param out The output stream to write the file to.
         */
        void write_graph(std::ostream& out);

        /**
         * Detects cycles within the graph.
         * Throws an evaluation exception if cycles are detected.
         */
        void detect_cycles();

     private:
        void populate_graph();
        void process_relationship_parameter(resource const& source, std::string const& name, runtime::relationship relationship);

        std::unordered_map<std::string, std::unordered_map<std::string, resource>> _resources;
        std::unordered_map<types::klass, std::vector<class_definition>, boost::hash<types::klass>> _classes;
        std::unordered_map<std::string, defined_type> _defined_types;
        std::vector<node_definition> _nodes;
        std::unordered_map<std::string, size_t> _named_nodes;
        std::vector<std::pair<values::regex, size_t>> _regex_nodes;
        boost::optional<size_t> _default_node_index;
        dependency_graph _graph;
    };

}}  // puppet::runtime
