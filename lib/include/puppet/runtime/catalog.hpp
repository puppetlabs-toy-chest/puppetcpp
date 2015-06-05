/**
 * @file
 * Declares the Puppet catalog.
 */
#pragma once

#include "../lexer/position.hpp"
#include "values/value.hpp"
#include <string>
#include <functional>
#include <unordered_set>
#include <unordered_map>

namespace puppet { namespace runtime {

    // Forward declaration for catalog.
    struct catalog;

    /**
     * Represents a resource in a catalog.
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

     private:
        std::unordered_map<std::string, std::unordered_map<std::string, resource>> _resources;
        std::unordered_map<std::string, std::unordered_map<std::string, resource*>> _aliases;
    };

}}  // puppet::runtime
