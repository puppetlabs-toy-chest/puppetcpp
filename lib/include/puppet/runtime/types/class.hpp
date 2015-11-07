/**
 * @file
 * Declares the class type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <string>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Class type.
     */
    struct klass
    {
        /**
         * Constructs a Class type.
         * @param title The title of the class (e.g. 'main').  If empty, represents all instances of the class type.
         */
        explicit klass(std::string title = {});

        /**
         * Gets the title of the class.
         * @return Returns the title of the class.
         */
        std::string const& title() const;

        /**
         * Determines if the class type is fully qualified.
         * @return Returns true if the class type is fully qualified or false if not.
         */
        bool fully_qualified() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Class).
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
         * Normalizes a class name.
         * @param name The class name to normalize.
         */
        static void normalize(std::string& name);

    private:
        std::string _title;
    };

    /**
     * Stream insertion operator for class type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, klass const& type);

    /**
     * Equality operator for class type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(klass const& left, klass const& right);

    /**
     * Inequality operator for class type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(klass const& left, klass const& right);

    /**
     * Hashes the klass type.
     * @param type The klass type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(klass const& type);

}}}  // namespace puppet::runtime::types
