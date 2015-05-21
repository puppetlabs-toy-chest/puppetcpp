/**
 * @file
 * Declares the Puppet catalog.
 */
#pragma once

#include "../lexer/token_position.hpp"
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
         * @param type The type of the resource (e.g. File).
         * @param title The title of the resource (e.g. '/tmp/foo').
         * @param file The file defining the resource.
         * @param line The line defining the resource.
         * @param exported True if the resource is exported or false if not.
         */
        resource(runtime::catalog& catalog, std::string type, std::string title, std::string file, size_t line, bool exported = false);

        /**
         * Gets the catalog that contains the resource.
         * @return Returns the catalog that contains the resource.
         */
        runtime::catalog const& catalog() const;

        /**
         * Gets the type of the resource.
         * @return Returns the type of the resource.
         */
        std::string const& type() const;

        /**
         * Gets the title of the resource.
         * @return Returns the title of the resource.
         */
        std::string const& title() const;

        /**
         * Gets the file where the resource was defined.
         * @return Returns the file where the resource was defined.
         */
        std::string const& file() const;

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
        void set_parameter(std::string const& name, lexer::token_position const& name_position, values::value value, lexer::token_position const& value_position, bool override = false);

        /**
         * Removes a parameter from the resource.
         * @param name The name of the parameter to remove.
         * @return Returns true if the parameter was removed or false if it did not exist.
         */
        bool remove_parameter(std::string const& name);

        /**
         * Creates a reference to this resource.
         * @return Returns a reference to this resource.
         */
        types::resource create_reference() const;

     private:
        void store_parameter(std::string const& name, lexer::token_position const& name_position, values::value value, bool override);
        bool handle_metaparameter(std::string const& name, lexer::token_position const& name_position, values::value& value, lexer::token_position const& value_position);
        void create_alias(values::value const& value, lexer::token_position const& position);

        runtime::catalog& _catalog;
        std::string _type;
        std::string _title;
        std::string _file;
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
         * Used for resource map equality.
         */
        struct type_equal_to : std::binary_function<std::string, std::string, bool>
        {
            /**
             * Compares two strings for case-insensitive equality.
             * @param left The left string to compare.
             * @param right The right string to compare.
             * @return Returns true if both strings are case-insensitively equal or false if not.
             */
            bool operator()(std::string const& left, std::string const& right) const
            {
                return boost::algorithm::iequals(left, right);
            }
        };

        /**
         * Used for resource map hashing.
         */
        struct type_hasher : std::unary_function<std::string, std::size_t>
        {
            /**
             * Hashes the given type string.
             * @param type The type string to hash.
             * @return Returns the hash value.
             */
            std::size_t operator()(std::string const& type) const
            {
                std::size_t seed = 0;

                for (char c : type)
                {
                    boost::hash_combine(seed, std::toupper(c));
                }
                return seed;
            }
        };

        /**
         * The type for resource map.
         */
        typedef std::unordered_map<std::string, std::unordered_map<std::string, resource>, type_hasher, type_equal_to> resource_map;
        /**
         * The type for resource alias map.
         */
        typedef std::unordered_map<std::string, std::unordered_map<std::string, resource*>, type_hasher, type_equal_to> resource_alias_map;

        /**
         * Constructs a catalog.
         */
        catalog();

        /**
         * Gets the resources currently in the catalog.
         * @return Returns the resources currently in the catalog.
         */
        resource_map const& resources() const;

        /**
         * Finds a resource in the catalog.
         * @param type The type of the resource.
         * @param title The title of the resource.
         * @return Returns the resource if found or nullptr if the resource is not in the catalog.
         */
        resource const* find_resource(std::string const& type, std::string const& title) const;

        /**
         * Finds a resource in the catalog.
         * @param type The type of the resource.
         * @param title The title of the resource.
         * @return Returns the resource if found or nullptr if the resource is not in the catalog.
         */
        resource* find_resource(std::string const& type, std::string const& title);

        /**
         * Aliases a resource.
         * @param type The type of the resource to alias.
         * @param title The title of the resource being aliased.
         * @param alias The new alias name.
         * @return Returns true if the resource was aliased or false if a resource with that name or alias already exists.
         */
        bool alias_resource(std::string const& type, std::string const& title, std::string const& alias);

        /**
         * Adds a resource to the catalog.
         * @param type The type of resource to add.
         * @param title The title of the resource to add.
         * @param file The file defining the resource.
         * @param line The line defining the resource.
         * @param exported True if the resource is exported or false if it is not.
         * @return Returns the new resource in the catalog or nullptr if the resource already exists in the catalog.
         */
        resource* add_resource(std::string const& type, std::string const& title, std::string const& file, size_t line, bool exported = false);

     private:
        resource_map _resources;
        resource_alias_map _aliases;
    };

}}  // puppet::runtime
