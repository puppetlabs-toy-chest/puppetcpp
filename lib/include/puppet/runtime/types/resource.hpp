/**
 * @file
 * Declares the resource type.
 */
#pragma once

#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>
#include <ostream>
#include <string>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Resource type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct basic_resource
    {
        /**
         * Constructs a Resource type.
         * @param type_name The type name of the resource (e.g. File).  If empty, represents all resources.
         * @param title The title of the resource (e.g. '/foo').  If empty, represents all instances of the resource type.
         */
        explicit basic_resource(std::string type_name = {}, std::string title = {}) :
            _type_name(std::move(type_name)),
            _title(std::move(title))
        {
            // Make the type name lowercase
            boost::to_lower(_type_name);

            // Now uppercase every start of a type name
            boost::split_iterator<std::string::iterator> end;
            for (auto it = boost::make_split_iterator(_type_name, boost::first_finder("::", boost::is_equal())); it != end; ++it) {
                auto range = boost::make_iterator_range(it->begin(), it->begin() + 1);
                boost::to_upper(range);
            }
        }

        /**
         * Gets the type name of the resource.
         * @return Returns the type of the resource.
         */
        std::string const& type_name() const
        {
            return _type_name;
        }

        /**
         * Gets the title of the resource.
         * @return Returns the title of the resource.
         */
        std::string const& title() const
        {
            return _title;
        }

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Resource).
         */
        static const char* name()
        {
            return "Resource";
        }

        /**
         * Determines if the given value is an instance of this type.
         * @tparam Value The type of the runtime value.
         * @param value The value to determine if it is an instance of this type. This value will never be a variable.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        template <typename Value>
        bool is_instance(Value const& value) const
        {
            // Check for type
            auto ptr = boost::get<Type>(&value);
            if (!ptr) {
                return false;
            }
            // Check for resource type
            auto resource_ptr = boost::get<basic_resource<Type>>(ptr);
            if (!resource_ptr) {
                return false;
            }
            // If no type, the given value is a 'resource'
            if (_type_name.empty()) {
                return true;
            }
            // Check type name
            if (_type_name != resource_ptr->type_name()) {
                return false;
            }
            return _title.empty() || _title == resource_ptr->title();
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(Type const& other) const
        {
            // If the resource does not have a specialization, the other type cannot be one
            if (_type_name.empty()) {
                return false;
            }
            // Check that the other Resource is specialized
            auto resource = boost::get<basic_resource<Type>>(&other);
            if (!resource || resource->type_name() != _type_name) {
                // Not the same type
                return false;
            }
            // Otherwise, the other one is a specialization if this does not have a title but the other one does
            return _title.empty() && !resource->title().empty();
        }

     private:
        std::string _type_name;
        std::string _title;
    };

    /**
     * Stream insertion operator for resource type.
     * @tparam Type The type of a runtime type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    template <typename Type>
    std::ostream& operator<<(std::ostream& os, basic_resource<Type> const& type)
    {
        if (type.type_name().empty()) {
            os << basic_resource<Type>::name();
            return os;
        }
        os << type.type_name();
        if (type.title().empty()) {
            return os;
        }
        os << "['" << type.title() << "']";
        return os;
    }

    /**
     * Equality operator for resource type.
     * @tparam Type The type of a runtime type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    template <typename Type>
    bool operator==(basic_resource<Type> const& left, basic_resource<Type> const& right)
    {
        return left.type_name() == right.type_name() && left.title() == right.title();
    }

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Resource type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct hash<puppet::runtime::types::basic_resource<Type>>
    {
        /**
         * Hashes the Resource type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::basic_resource<Type> const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::basic_resource<Type>::name());
            hash_combine(seed, type.type_name());
            hash_combine(seed, type.title());
            return seed;
        }
    };
}
