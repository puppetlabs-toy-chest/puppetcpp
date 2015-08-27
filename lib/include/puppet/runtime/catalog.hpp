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
     * Represents a resource attribute.
     */
    struct attribute
    {
        /**
         * Constructs a resource attribute.
         * @param context The compilation context where the attribute was set.
         * @param name The name of the attribute.
         * @param name_position The position of the attribute's name.
         * @param value The attribute's value.
         * @param value_position The position of the attribute's value.
         */
        attribute(
            std::shared_ptr<compiler::context> context,
            std::string name,
            lexer::position name_position,
            std::shared_ptr<values::value> value,
            lexer::position value_position);

        /**
         * Gets the compilation context where the attribute was set.
         * @return Returns the compilation context where the attribute was set.
         */
        std::shared_ptr<compiler::context> const& context() const;

        /**
         * Gets the name of the attribute.
         * @return Returns the name of the attribute.
         */
        std::string const& name() const;

        /**
         * Gets the position of the attribute's name.
         * @return Returns the position of the attribute's name.
         */
        lexer::position const& name_position() const;

        /**
         * Gets the attribute's value.
         * @return Returns the attribute's value.
         */
        std::shared_ptr<values::value> const& value() const;

        /**
         * Gets the position of the attribute's value.
         * @return Returns the position of the attribute's value.
         */
        lexer::position const& value_position() const;

    private:
        std::shared_ptr<compiler::context> _context;
        std::string _name;
        lexer::position _name_position;
        std::shared_ptr<values::value> _value;
        lexer::position _value_position;
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
         * @param context The compilation context where the resource was declared.
         * @param position The position where the resource was declared.
         * @param exported True if the resource is exported or false if not.
         */
        resource(
            types::resource type,
            std::shared_ptr<compiler::context> context,
            lexer::position position,
            bool exported = false);

        /**
         * Gets the resource type of the resource.
         * @return Returns the resource type of the resource.
         */
        types::resource const& type() const;

        /**
         * Gets the compilation context where the resource was declared.
         * Note: the compilation context will be reset after the resource is evaluated.
         * @return Returns the compilation context where the resource was declared.
         */
        std::shared_ptr<compiler::context> const& context() const;

        /**
         * Gets the position where the resource was declared.
         * @return Returns the position where the resource was declared.
         */
        lexer::position const& position() const;

        /**
         * Gets the path of the file where the resource was declared.
         * @return Returns the path of the file where the resource was declared.
         */
        std::string const& path() const;

        /**
         * Gets whether or not the resource is exported.
         * @return Returns true if the resource is exported or false if it is not.
         */
        bool exported() const;

        /**
         * Sets an attribute on the resource.
         * @param attribute The attribute to set on the resource.
         */
        void set(std::shared_ptr<runtime::attribute> attribute);

        /**
         * Appends an attribute on the resource.
         * If the attribute already exists as an array, the new value is appended to the old.
         * @param attribute The attribute to append on the resource.
         */
        void append(std::shared_ptr<runtime::attribute> attribute);

        /**
         * Gets an attribute on the resource.
         * @param name The name of the attribute to get.
         * @return Returns a resource of the given name or nullptr if no attribute exists.
         */
        std::shared_ptr<attribute> get(std::string const& name) const;

        /**
         * Enumerates each attribute in the resource.
         * @param callback The callback to call for each attribute.
         */
        void each_attribute(std::function<bool(attribute const&)> const& callback) const;

        /**
         * Gets the vertex id of the resource in the catalog dependency graph.
         * @return Returns the vertex id of the resource in the catalog dependency graph.
         */
        size_t vertex_id() const;

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
        std::shared_ptr<compiler::context> _context;
        std::shared_ptr<std::string> _path;
        lexer::position _position;
        std::unordered_map<std::string, std::shared_ptr<attribute>> _attributes;
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
         * @return Returns the path of the file containing the class definition.
         */
        std::string const& path() const;

        /**
         * Gets the position of the class definition.
         * @return Returns the position of the class definition.
         */
        lexer::position const& position() const;

     private:
        friend struct catalog;
        void evaluate(runtime::context& context, runtime::resource& resource) const;
        std::shared_ptr<runtime::scope> evaluate_parent(runtime::context& context) const;

        types::klass _klass;
        boost::optional<types::klass> _parent;
        std::shared_ptr<compiler::context> _context;
        ast::class_definition_expression const& _expression;
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
         * Gets the path of the file containing the class definition.
         * @return Returns the path of the file containing the class definition.
         */
        std::string const& path() const;

        /**
         * Gets the position of the class definition.
         * @return Returns the position of the class definition.
         */
        lexer::position const& position() const;

     private:
        friend struct catalog;
        void evaluate(runtime::context& context, runtime::resource& resource) const;

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
        void evaluate(runtime::context& context, runtime::resource& resource) const;

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
        void add_relationship(runtime::relationship relationship, resource const& source, resource const& target);

        /**
         * Finds a resource in the catalog.
         * @param resource The resource type to find.
         * @return Returns the resource if found or nullptr if the resource is not in the catalog.
         */
        resource* find_resource(types::resource const& resource);

        /**
         * Adds a resource to the catalog.
         * @param type The qualified resource type to add.
         * @param compilation_context The compilation context where the resource is being declared.
         * @param position The position where the resource is being declared.
         * @param container The container of the resource or nullptr for no container.
         * @param exported True if the resource is exported or false if it is not.
         * @return Returns the resource that was added.
         */
        resource& add_resource(
            types::resource type,
            std::shared_ptr<compiler::context> const& compilation_context,
            lexer::position const& position,
            resource const* container = nullptr,
            bool exported = false);

        /**
         * Finds the definitions of a class.
         * @param klass The class to find.
         * @param node The node to load the class from.  If nullptr, the class will not be loaded.
         * @return Returns the class definitions if defined or nullptr if not defined.
         */
        std::vector<class_definition> const* find_class(types::klass const& klass, compiler::node const* node = nullptr);

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
         * If the class is already declared, the existing class is returned.
         * @param evaluation_context The current evaluation context.
         * @param type The qualified resource type for the class.
         * @param compilation_context The compilation context where the class is being declared.
         * @param position The position where the class is being declared.
         * @return Returns the declared class resource.
         */
        resource& declare_class(
            runtime::context& evaluation_context,
            types::resource const& type,
            std::shared_ptr<compiler::context> const& compilation_context,
            lexer::position const& position);

        /**
         * Finds a defined type's definition.
         * @param type The type name of the defined type.  Note: this is the lower case type name as used in a manifest, e.g. "foo::bar".
         * @return Returns the defined type's definition if defined or nullptr if not defined.
         */
        defined_type const* find_defined_type(std::string const& type);

        /**
         * Defines a defined type.
         * Only one definition of a defined type may exist.
         * Defined types are declared like any other resources and are evaluated when the catalog is finalized.
         * @param type The name of the defined type.
         * @param context The compilation context where the defined type is defined.
         * @param expression The defined type expression.
         */
        void define_type(
            std::string type,
            std::shared_ptr<compiler::context> const& context,
            ast::defined_type_expression const& expression);

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
        resource* declare_node(runtime::context& evaluation_context, compiler::node const& node);

        /**
         * Finalizes the catalog.
         * Generates resources and populates the dependency graph.
         * @param context The current evaluation context.
         */
        void finalize(runtime::context& context);

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
        void evaluate_defined_types(runtime::context& context);

        std::unordered_map<std::string, std::unordered_map<std::string, resource>> _resources;
        std::unordered_map<types::klass, std::vector<class_definition>, boost::hash<types::klass>> _classes;
        std::unordered_set<std::string> _declared_classes;
        std::unordered_map<std::string, defined_type> _defined_types;
        std::vector<std::pair<defined_type const*, resource*>> _declared_defined_types;
        std::vector<node_definition> _nodes;
        std::unordered_map<std::string, size_t> _named_nodes;
        std::vector<std::pair<values::regex, size_t>> _regex_nodes;
        boost::optional<size_t> _default_node_index;
        dependency_graph _graph;
    };

}}  // puppet::runtime
