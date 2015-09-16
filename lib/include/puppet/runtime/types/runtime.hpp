/**
 * @file
 * Declares the "runtime" type.
 */
#pragma once

#include "../../cast.hpp"
#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Runtime type.
     */
    struct runtime
    {
        /**
         * Constructs a Runtime type.
         * @param runtime_name The name of the runtime (e.g. C++).
         * @param type_name The name of the type (e.g. QueryCollector).
         */
        explicit runtime(std::string runtime_name = {}, std::string type_name = {}) :
            _runtime_name(rvalue_cast(runtime_name)),
            _type_name(rvalue_cast(type_name))
        {
        }

        /**
         * Gets the runtime name.
         * @return Returns the runtime name.
         */
        std::string const& runtime_name() const
        {
            return _runtime_name;
        }

        /**
         * Gets the type name.
         * @return Returns the type name.
         */
        std::string const& type_name() const
        {
            return _type_name;
        }

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Runtime).
         */
        static const char* name();

        /**
         * Determines if the given value is an instance of this type.
         * @tparam Value The type of the runtime value.
         * @param value The value to determine if it is an instance of this type. This value will never be a variable.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        template <typename Value>
        bool is_instance(Value const& value) const
        {
            // No values are instances of runtime objects
            return false;
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @tparam Type The type of runtime type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        template <typename Type>
        bool is_specialization(Type const& other) const
        {
            // Check that the other Runtime is specialized
            auto type = boost::get<runtime>(&other);
            if (!type) {
                // Not the same type
                return false;
            }
            // If this Runtime object has no runtime name, the other is specialized if it does have one
            if (_runtime_name.empty()) {
                return !type->runtime_name().empty();
            }
            // Otherwise, the runtimes need to be the same
            if (_runtime_name != type->runtime_name()) {
                return false;
            }
            // Otherwise, the other one is a specialization if this does not have a type but the other one does
            return _type_name.empty() && !type->type_name().empty();
        }

     private:
        std::string _runtime_name;
        std::string _type_name;
    };

    /**
     * Stream insertion operator for runtime type.
     * @param os The output stream to write the type to.
     * @param type The Runtime type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, runtime const& type);

    /**
     * Equality operator for runtime.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(runtime const& left, runtime const& right);

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Runtime type.
     */
    template <>
    struct hash<puppet::runtime::types::runtime>
    {
        /**
         * Hashes the Runtime type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::runtime const& type) const
        {
            static const size_t name_hash = boost::hash_value(puppet::runtime::types::runtime::name());

            size_t seed = 0;
            hash_combine(seed, name_hash);
            hash_combine(seed, type.runtime_name());
            hash_combine(seed, type.type_name());
            return seed;
        }
    };
}
