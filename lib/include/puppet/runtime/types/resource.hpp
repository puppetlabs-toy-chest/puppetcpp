/**
 * @file
 * Declares the resource type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <string>
#include <boost/optional.hpp>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Resource type.
     */
    struct resource
    {
        /**
         * Constructs a Resource type.
         * @param type_name The type name of the resource (e.g. File).  If empty, represents all resources.
         * @param title The title of the resource (e.g. '/foo').  If empty, represents all instances of the resource type.
         */
        explicit resource(std::string type_name = {}, std::string title = {});

        /**
         * Gets the type name of the resource.
         * @return Returns the type of the resource.
         */
        std::string const& type_name() const;

        /**
         * Gets the title of the resource.
         * @return Returns the title of the resource.
         */
        std::string const& title() const;

        /**
         * Determines if the resource type is fully qualified.
         * @return Returns true if the resource type is fully qualified or false if not.
         */
        bool fully_qualified() const;

        /**
         * Determines if the resource is a class.
         * @return Returns true if the resource is a class or false if not.
         */
        bool is_class() const;

        /**
         * Determines if the resource is a stage.
         * @return Returns true if the resource is a stage or false if not.
         */
        bool is_stage() const;

        /**
         * Determines if the resource is a "built-in" type.
         * @return Returns true if the resource is a "built-in" type or false if not.
         */
        bool is_builtin() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Resource).
         */
        static char const* name();

        /**
         * Determines if the given value is an instance of this type.
         * @param value The value to determine if it is an instance of this type.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        bool is_instance(values::value const& value) const;

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(values::type const& other) const;

        /**
         * Parses a resource type specification into a resource.
         * @param specification The resource type specification to parse (e.g. File[foo]).
         * @return Returns the resource type if successful or boost::none if parsing was unsuccessful.
         */
        static boost::optional<resource> parse(std::string const& specification);

     private:
        std::string _type_name;
        std::string _title;
    };

    /**
     * Stream insertion operator for resource type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource const& type);\

    /**
     * Equality operator for resource type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(resource const& left, resource const& right);

    /**
     * Inequality operator for resource type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(resource const& left, resource const& right);

    /**
     * Hashes the resource type.
     * @param type The resource type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(resource const& type);

}}}  // namespace puppet::runtime::types
