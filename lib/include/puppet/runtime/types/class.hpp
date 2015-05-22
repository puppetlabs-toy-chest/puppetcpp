/**
 * @file
 * Declares the class type.
 */
#pragma once

#include "../../cast.hpp"
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>
#include <ostream>
#include <string>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Class type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct basic_class
    {
        /**
         * Constructs a Class type.
         * @param title The title of the class (e.g. 'main').  If empty, represents all instances of the class type.
         */
        explicit basic_class(std::string title = {}) :
            _title(rvalue_cast(title))
        {
        }

        /**
         * Gets the title of the class.
         * @return Returns the title of the class.
         */
        std::string const& title() const
        {
            return _title;
        }

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Class).
         */
        static const char* name()
        {
            return "Class";
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
            // Check for class type
            auto class_ptr = boost::get<basic_class<Type>>(ptr);
            if (!class_ptr) {
                return false;
            }
            return _title.empty() || _title == class_ptr->title();
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(Type const& other) const
        {
            // If the class has a specialization, the other type cannot be one
            if (!_title.empty()) {
                return false;
            }
            // Check that the other Class is specialized
            auto class_ptr = boost::get<basic_class<Type>>(&other);
            if (!class_ptr) {
                // Not the same type
                return false;
            }
            // Otherwise, the other one is a specialization if it has a title
            return !class_ptr->title().empty();
        }

    private:
        std::string _title;
    };

    /**
     * Stream insertion operator for class type.
     * @tparam Type The type of a runtime type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    template <typename Type>
    std::ostream& operator<<(std::ostream& os, basic_class<Type> const& type)
    {
        os << basic_class<Type>::name();
        if (type.title().empty()) {
            return os;
        }
        os << "[" << type.title() << "]";
        return os;
    }

    /**
     * Equality operator for class type.
     * @tparam Type The type of a runtime type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    template <typename Type>
    bool operator==(basic_class<Type> const& left, basic_class<Type> const& right)
    {
        return left.title() == right.title();
    }

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Class type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct hash<puppet::runtime::types::basic_class<Type>>
    {
        /**
         * Hashes the Class type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::basic_class<Type> const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::basic_class<Type>::name());
            hash_combine(seed, type.title());
            return seed;
        }
    };
}
