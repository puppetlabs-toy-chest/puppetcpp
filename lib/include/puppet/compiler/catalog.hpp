/**
 * @file
 * Declares the catalog.
 */
#pragma once

#include "resource.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <deque>

namespace puppet { namespace compiler {

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
     * @param relation The relationship to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& out, relationship relation);

    /**
     * Represents an exception when a resource cycle is detected.
     */
    struct resource_cycle_exception : std::runtime_error
    {
        /**
         * Constructs a resource cycle exception.
         * @param message The exception message.
         */
        explicit resource_cycle_exception(std::string const& message);
    };

    /**
     * Represents the Puppet catalog.
     */
    struct catalog
    {
        /**
         * Constructs a catalog given the node and environment names.
         * @param node The node name.
         * @param environment The environment name.
         */
        catalog(std::string node, std::string environment);

        /**
         * Default move constructor for catalog.
         */
        catalog(catalog&&) = default;

        /**
         * Default move assignment operator for catalog.
         * @return Returns this catalog.
         */
        catalog& operator=(catalog&&) = default;

        /**
         * Gets the name of the node this catalog was compiled for.
         * @return Returns the node name.
         */
        std::string const& node() const;

        /**
         * Gets the name of the environment this catalog was compiled for.
         * @return Returns the environment name.
         */
        std::string const& environment() const;

        /**
         * Adds a resource to the catalog.
         * @param type The resource type to add.
         * @param container The container of the resource.
         * @param context The AST context where the resource was declared.
         * @param virtualized True if the resource is virtualized or false if the resource should be realized.
         * @param exported True if the resource should be exported or false if not.
         * @return Returns a pointer to the resource that was added to the catalog or nullptr if the resource already exists.
         */
        resource* add(
            runtime::types::resource type,
            resource const* container = nullptr,
            ast::context const* context = nullptr,
            bool virtualized = false,
            bool exported = false);

        /**
         * Finds a resource in the catalog.
         * @param type The resource type to find.
         * @return Returns the resource if found or nullptr if the resource is not in the catalog.
         */
        resource* find(runtime::types::resource const& type);

        /**
         * Finds a resource in the catalog.
         * @param type The resource type to find.
         * @return Returns the resource if found or nullptr if the resource is not in the catalog.
         */
        resource const* find(runtime::types::resource const& type) const;

        /**
         * Gets the number of resources in the catalog.
         * @return Returns the number of resources in the catalog.
         */
        size_t size() const;

        /**
         * Enumerates the resources in the catalog.
         * @param callback The callback to call for each resource.
         * @param type The type to filter on; if empty, all resources are enumerated.
         * @param offset The offset to start from.
         */
        void each(std::function<bool(resource&)> const& callback, std::string const& type = std::string(), size_t offset = 0);

        /**
         * Enumerates the resources in the catalog.
         * @param callback The callback to call for each resource.
         * @param type The type to filter on; if empty, all resources are enumerated.
         * @param offset The offset to start from.
         */
        void each(std::function<bool(resource const&)> const& callback, std::string const& type = std::string(), size_t offset = 0) const;

        /**
         * Enumerates the dependency (out) edges of the given resource.
         * @param resource The resource to enumerate dependency edges for.
         * @param callback The callback to call for each dependency edge.
         */
        void each_edge(compiler::resource const& resource, std::function<bool(relationship, compiler::resource const&)> const& callback) const;

        /**
         * Adds a relationship (i.e. an edge) to the dependency graph.
         * The source will become dependent upon the target (reversed for before and notify relationships).
         * @param relation The relationship from the source to the target.
         * @param source The source resource.
         * @param target The target resource.
         */
        void relate(relationship relation, resource const& source, resource const& target);

        /**
         * Realizes a virtual resource.
         * Note: if the resource is already realized, this is a no-op.
         * @param resource The resource to realize.
         */
        void realize(compiler::resource& resource);

        /**
         * Populate's the catalog's graph with relationships from resource metaparameters.
         */
        void populate_graph();

        /**
         * Writes the catalog as JSON.
         * @param out The output stream to write the catalog to.
         */
        void write(std::ostream& out) const;

        /**
         * Writes the dependency graph as a DOT file.
         * @param out The output stream to write the file to.
         */
        void write_graph(std::ostream& out);

        /**
         * Detects cycles within the graph.
         * Throws a resource_cycle_exception if cycles are detected.
         */
        void detect_cycles();

     private:
        catalog(catalog&) = delete;
        catalog& operator=(catalog&) = delete;
        void populate_relationships(resource const& source, std::string const& name, compiler::relationship relationship);

        std::string _node;
        std::string _environment;
        // Use a deque to store the resources because deque doesn't invalidate references on push back
        // This enables us to store pointers to resources in various data structures and the dependency graph
        std::deque<resource> _resources;
        std::unordered_map<runtime::types::resource, resource*, boost::hash<runtime::types::resource>> _resource_map;
        std::unordered_map<std::string, std::vector<resource*>> _resource_lists;
        boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, resource*, relationship> _graph;
    };

}}  // namespace puppet::compiler
