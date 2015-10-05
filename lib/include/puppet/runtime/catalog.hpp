/**
 * @file
 * Declares the Puppet catalog.
 */
#pragma once

#include "../lexer/position.hpp"
#include "../ast/syntax_tree.hpp"
#include "collectors/collector.hpp"
#include "values/value.hpp"
#include <boost/optional.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <string>
#include <vector>
#include <deque>
#include <list>
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

    // Forward declaration of resource.
    struct resource;

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
     * Represents a resource dependency graph.
     */
    using dependency_graph = boost::adjacency_list<boost::multisetS, boost::vecS, boost::directedS, resource*, relationship>;

    /**
     * Represents a list of attributes paired with the attribute operator.
     */
    using attributes = std::vector<std::pair<ast::attribute_operator, std::shared_ptr<attribute>>>;

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
         * @param container The container of this resource; expected to be nullptr for stages and classes (class containment is explicit).
         * @param exported True if the resource is exported or false if not.
         */
        resource(
            types::resource type,
            std::shared_ptr<compiler::context> context,
            lexer::position position,
            runtime::resource const* container = nullptr,
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
         * Gets the container of this resource.
         * @return Returns the container of this resource.
         */
        runtime::resource const* container() const;

        /**
         * Gets whether or not this resource is virtual.
         * @return Returns true if the resource is virtual or false if it is realized.
         */
        bool virtualized() const;

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
         * Sets the given attributes.
         * @param attributes The attributes to set.
         * @param override True if existing attributes can be overriden or false if not.
         */
        void set(runtime::attributes const& attributes, bool override = false);

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
         * @return Returns the vertex id of the resource in the catalog dependency graph or numeric_limits<size_t>::max() if the resource is virtual.
         */
        size_t vertex_id() const;

        /**
         * Creates a RapidJSON value for this resource.
         * @param allocator The current RapidJSON allocator.
         * @param graph The dependency graph to source relationships from.
         * @return Returns the resources as a RapidJSON value.
         */
        rapidjson::Value to_json(rapidjson::Allocator& allocator, dependency_graph const& graph) const;

        /**
         * Determines if the given name is a metaparameter name.
         * @param name The name to check.
         * @return Returns true if name is the name of a metaparameter or false if not.
         */
        static bool is_metaparameter(std::string const& name);

     private:
        friend struct catalog;
        void vertex_id(size_t id);
        void write_relationship_parameters(rapidjson::Value& parameters, rapidjson::Allocator& allocator, dependency_graph const& graph) const;

        types::resource _type;
        std::shared_ptr<compiler::context> _context;
        std::shared_ptr<std::string> _path;
        lexer::position _position;
        runtime::resource const* _container;
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
     * Represents a resource override.
     * Resource overrides are applied immediately, upon resource declaration, or during catalog finalization.
     */
    struct resource_override
    {
        /**
         * Constructs a resource override.
         * @param context The context for the resource override.
         * @param position The position of the resource override.
         * @param type The resource type being overridden.
         * @param attributes The attributes being overridden.
         * @param scope The scope where the override is taking place.
         */
        resource_override(
            std::shared_ptr<compiler::context> context,
            lexer::position position,
            types::resource type,
            runtime::attributes attributes = runtime::attributes(),
            std::shared_ptr<runtime::scope> scope = nullptr);

        /**
         * Gets the context for the resource override.
         * @return Returns the context for the resource override.
         */
        std::shared_ptr<compiler::context> const& context() const;

        /**
         * Gets the position of the resource override.
         * @return Returns the position of the resource override.
         */
        lexer::position const& position() const;

        /**
         * Gets the resource type being overridden.
         * @return Returns the resource type being overridden.
         */
        types::resource const& type() const;

        /**
         * Gets the attributes being overridden.
         * @return Returns the attributes being overridden.
         */
        runtime::attributes const& attributes() const;

        /**
         * Gets the scope where the override is taking place.
         * @return Returns the scope where the override is taking place.
         */
        std::shared_ptr<runtime::scope> const& scope() const;

     private:
        friend struct catalog;
        void evaluate(runtime::catalog& catalog) const;

        std::shared_ptr<compiler::context> _context;
        lexer::position _position;
        types::resource _type;
        runtime::attributes _attributes;
        std::shared_ptr<runtime::scope> _scope;
    };

    /**
     * Represents a resource relationship resulting from a relationship operator.
     * Resource relationships are evaluated when a catalog is finalized.
     */
    struct resource_relationship
    {
        /**
         * Constructs a resource relationship.
         * @param context The compilation context for the relationship.
         * @param source The value representing the source.
         * @param source_position The position of the source.
         * @param target The value representing the target.
         * @param target_position The position of the target.
         * @param relationship The relationship between the source and the target.
         */
        resource_relationship(
            std::shared_ptr<compiler::context> context,
            values::value source,
            lexer::position source_position,
            values::value target,
            lexer::position target_position,
            runtime::relationship relationship);

        /**
         * Gets the compilation context for the relationship.
         * @return Returns the compilation context for the relationship.
         */
        std::shared_ptr<compiler::context> const& context() const;

        /**
         * Gets the source value.
         * @return Returns the source value.
         */
        values::value const& source() const;

        /**
         * Gets the position of the source.
         * @return Returns the position of the source.
         */
        lexer::position const& source_position() const;

        /**
         * Gets the target value.
         * @return Returns the target value.
         */
        values::value const& target() const;

        /**
         * Gets the position of the target.
         * @return Returns the position of the target.
         */
        lexer::position const& target_position() const;

        /**
         * Gets the relationship between the source and the target.
         * @return Returns the relationship between the source and the target.
         */
        runtime::relationship relationship() const;

     private:
        friend struct catalog;
        void evaluate(runtime::catalog& catalog) const;

        std::shared_ptr<compiler::context> _context;
        values::value _source;
        lexer::position _source_position;
        values::value _target;
        lexer::position _target_position;
        runtime::relationship _relationship;
    };

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
         * Adds a resource relationship.
         * Resource relationships are processed upon catalog finalization.
         * @param relationship The relationship to add.
         */
        void add_relationship(resource_relationship relationship);

        /**
         * Finds a resource in the catalog.
         * @param type The resource type to find.
         * @return Returns the resource if found or nullptr if the resource is not in the catalog.
         */
        resource* find_resource(types::resource const& type) const;

        /**
         * Finds resources in the catalog of a particular type.
         * @param type_name The type name to find the resources of.
         * @return Returns a vector of the resources of the type or nullptr if no resources exist of that type.
         */
        std::vector<resource*> const* find_resources(std::string const& type_name) const;

        /**
         * Adds a resource to the catalog.
         * @param type The qualified resource type to add.
         * @param compilation_context The compilation context where the resource is being declared.
         * @param position The position where the resource is being declared.
         * @param container The container of the resource or nullptr for no container.
         * @param virtualized True if the resource is virtualized or false if it not.
         * @param exported True if the resource is exported or false if it is not.
         * @param definition The associated defined type definition to run during catalog finalization.
         * @return Returns the resource that was added.
         */
        resource& add_resource(
            types::resource type,
            std::shared_ptr<compiler::context> const& compilation_context,
            lexer::position const& position,
            resource const* container = nullptr,
            bool virtualized = false,
            bool exported = false,
            defined_type const* definition = nullptr);

        /**
         * Realizes a virtual resource.
         * Note: if the resource is already realized, this is a no-op.
         * @param resource The resource to realize.
         */
        void realize(runtime::resource& resource);

        /**
         * Adds a resource override.
         * If the resource does not exist yet, the override will be evaluated upon declaration or catalog finalization.
         * If the resource does exist, the override will be evaluated immediately.
         * @param override The resource override.
         */
        void add_override(resource_override override);

        /**
         * Evaluates the overrides for a resource.
         * If the resource does not exist, an evaluation exception will be thrown.
         * @param type The resource type to evaluate the overrides for.
         */
        void evaluate_overrides(types::resource const& type);

        /**
         * Finds the definitions of a class.
         * @param klass The class to find.
         * @param context The evaluation context to load a class into.  If nullptr, no classes will be loaded.
         * @return Returns the class definitions if defined or nullptr if not defined.
         */
        std::vector<class_definition> const* find_class(types::klass const& klass, runtime::context* context = nullptr);

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
         * @param context The evaluation context to load a defined type.  If nullptr, no defined types will be loaded.
         * @return Returns the defined type's definition if defined or nullptr if not defined.
         */
        defined_type const* find_defined_type(std::string const& type, runtime::context* context = nullptr);

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
         * Adds a collector to the catalog.
         * @param collector The collector to add.
         */
        void add_collector(std::shared_ptr<collectors::collector> collector);

        /**
         * Finalizes the catalog.
         * Generates resources and populates the dependency graph.
         * @param context The current evaluation context.
         */
        void finalize(runtime::context& context);

        /**
         * Writes the catalog as JSON.
         * @param node The node associated with the catalog.
         * @param out The output stream to write the JSON to.
         */
        void write(compiler::node const& node, std::ostream& out) const;

        /**
         * Writes the dependency graph as a DOT file.
         * @param out The output stream to write the file to.
         */
        void write_graph(std::ostream& out);

        /**
         * Detects cycles within the graph.
         * Throws a compilation_exception if cycles are detected.
         */
        void detect_cycles();

     private:
        void populate_graph();
        void process_relationship_parameter(resource const& source, std::string const& name, runtime::relationship relationship);
        void evaluate_defined_types(runtime::context& context, size_t& index, std::list<std::pair<defined_type const*, resource*>>& virtualized);

        // Stores the resources in declaration order
        // Note: this is a deque because we need references to not be invalidated when inserting to the end
        std::deque<resource> _resources;
        // Stores a mapping between qualified resource type (e.g. Foo[bar]) and resource
        std::unordered_map<types::resource, resource*, boost::hash<types::resource>> _resource_map;
        // Stores a mapping between type name (e.g. Foo) and declared resources of that type, in declaration order for resources of that type
        std::unordered_map<std::string, std::vector<resource*>> _resource_lists;
        // Stores a mapping between class and definitions in declaration order
        std::unordered_map<types::klass, std::vector<class_definition>, boost::hash<types::klass>> _class_definitions;
        // Stores the set of declared classes in the catalog
        std::unordered_set<std::string> _classes;
        // Stores the mapping between defined type name (e.g. foo::bar) and the defined type definition
        std::unordered_map<std::string, defined_type> _defined_type_definitions;
        // Stores the declared defined types in declaration order
        std::vector<std::pair<defined_type const*, resource*>> _defined_types;
        // Stores the node definitions in declaration order
        std::vector<node_definition> _node_definitions;
        // Stores the mapping between a node name and the index into the node definitions list
        std::unordered_map<std::string, size_t> _named_nodes;
        // Stores the node regexes in declaration order, paired with the index into the node definition list
        std::vector<std::pair<values::regex, size_t>> _regex_nodes;
        // Stores the default index into the node definitions list
        boost::optional<size_t> _default_node_index;
        // Stores the delayed resource overrides
        std::unordered_multimap<types::resource, resource_override, boost::hash<types::resource>> _overrides;
        // Stores the resource relationships processed at finalization
        std::vector<resource_relationship> _relationships;
        // Stores the collectors
        std::vector<std::shared_ptr<collectors::collector>> _collectors;
        // Stores the resource dependency graph
        dependency_graph _graph;
    };

}}  // puppet::runtime
