/**
 * @file
 * Declares the catalog resource.
 */
#pragma once

#include "attribute.hpp"
#include <string>
#include <memory>
#include <functional>
#include <set>
#include <unordered_map>

namespace puppet { namespace compiler {

    // Forward declaration of catalog.
    struct catalog;

    /**
     * Utility class for tag_set.
     */
    struct indirect_less
    {
        /**
         * Determines if one indirected string is "less than" another.
         * @param left The left string to compare.
         * @param right The right string to compare.
         * @return Returns true if the left is less than the right or false if not.
         */
        bool operator()(std::string const* left, std::string const* right) const;
    };

    /**
     * Represents a set of tags (pointers to data stored within each resource).
     */
    using tag_set = std::set<std::string const*, indirect_less>;

    /**
     * Represents a declared resource in a catalog.
     * Resources are created through catalog::add.
     */
    struct resource
    {
        /**
         * Gets the resource type of the resource.
         * @return Returns the resource type of the resource.
         */
        runtime::types::resource const& type() const;

        /**
         * Gets the container of the resource.
         * @return Returns the container of the resource.
         */
        resource const* container() const;

        /**
         * Gets the context where the resource was declared.
         * @return Returns the context where the resource was declared.
         */
        ast::context const& context() const;

        /**
         * Gets the path of the file where the resource was declared.
         * @return Returns the path of the file where the resource was declared.
         */
        std::string const& path() const;

        /**
         * Gets the line of the file where the resource was declared.
         * @return Returns the line of the file where the resource was declared.
         */
        size_t line() const;

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
         * Gets an attribute on the resource.
         * @param name The name of the attribute to get.
         * @return Returns a resource of the given name or nullptr if no attribute exists.
         */
        std::shared_ptr<attribute> get(std::string const& name) const;

        /**
         * Sets an attribute on the resource.
         * @param attribute The attribute to set on the resource.
         */
        void set(std::shared_ptr<compiler::attribute> attribute);

        /**
         * Appends an attribute on the resource.
         * If the attribute already exists as an array, the new value is appended to the old.
         * @param attribute The attribute to append on the resource.
         * @return Returns false if the existing value is not an array.
         */
        bool append(std::shared_ptr<compiler::attribute> attribute);

        /**
         * Applies the given attributes to the resource.
         * @param attributes The attributes to apply.
         * @param override True if existing attributes can be overriden or false if not.
         */
        void apply(compiler::attributes const& attributes, bool override = false);

        /**
         * Enumerates each attribute in the resource.
         * @param callback The callback to call for each attribute.
         */
        void each_attribute(std::function<bool(attribute const&)> const& callback) const;

        /**
         * Tags the resource with the given tag.
         * @param tag The string to tag the resource with.
         */
        void tag(std::string tag);

        /**
         * Calculates the tags for the resource.
         * @return Returns the tag set for the resource.
         */
        tag_set calculate_tags() const;

        /**
         * Determines if the given name is a metaparameter name.
         * @param name The name to check.
         * @return Returns true if name is the name of a metaparameter or false if not.
         */
        static bool is_metaparameter(std::string const& name);

     private:
        friend struct catalog;

        resource(runtime::types::resource type, resource const* container, boost::optional<ast::context> context, bool exported);
        runtime::values::json_value to_json(runtime::values::json_allocator& allocator, compiler::catalog const& catalog) const;
        void add_relationship_parameters(runtime::values::json_value& parameters, runtime::values::json_allocator& allocator, compiler::catalog const& catalog) const;
        void realize(size_t vertex_id);
        size_t vertex_id() const;
        void populate_tags(tag_set& tags) const;

        std::shared_ptr<ast::syntax_tree> _tree;
        runtime::types::resource _type;
        resource const* _container;
        boost::optional<ast::context> _context;
        size_t _vertex_id;
        std::unordered_map<std::string, std::shared_ptr<attribute>> _attributes;
        std::vector<std::string> _tags;
        bool _exported;
    };

}}  // namespace puppet::compiler
