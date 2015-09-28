/**
 * @file
 * Declares the "runtime" type.
 */
#pragma once

#include "../../cast.hpp"
#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace collectors {

    // Forward declaration of collector.
    struct collector;

}}}  // namespace puppet::runtime::collectors

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Runtime type.
     * This type represents an object in the runtime that is not part of the Puppet type system.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct basic_runtime
    {
        /**
         * The type of associated object.
         */
        using object_type =
            boost::variant<
                std::shared_ptr<collectors::collector>
            >;

        /**
         * Helper for converting runtime object to a "type name".
         */
        struct type_name_visitor : boost::static_visitor<std::string>
        {
            /**
             * Converts a collector to a type name string.
             * @return Returns the type name for collector types.
             */
            result_type operator()(std::shared_ptr<collectors::collector> const&) const
            {
                return "Collector";
            }
        };

        /**
         * Constructs a Runtime type.
         * @param runtime_name The name of the runtime (e.g. C++).
         * @param type_name The name of the type (e.g. Collector).
         */
        explicit basic_runtime(std::string runtime_name = {}, std::string type_name = {}) :
            _runtime_name(rvalue_cast(runtime_name)),
            _type_name(rvalue_cast(type_name))
        {
        }

        /**
         * Constructs a Runtime type.
         * @param object The runtime object associated with this type.
         */
        explicit basic_runtime(boost::optional<object_type> object) :
            _runtime_name("C++"),
            _object(rvalue_cast(object))
        {
            if (_object) {
                _type_name = boost::apply_visitor(type_name_visitor(), *object);
            }
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
         * Gets the associated object.
         * @return Returns the associated object.
         */
        boost::optional<object_type> const& object() const
        {
            return _object;
        }

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Runtime).
         */
        static const char* name()
        {
            return "Runtime";
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
            auto type = boost::get<Type>(&value);
            if (!type) {
                return false;
            }
            // Check for runtime type
            auto runtime = boost::get<basic_runtime<Type>>(type);
            if (!runtime) {
                return false;
            }
            // If no runtime specified, then the value is a "runtime"
            if (_runtime_name.empty()) {
                return true;
            }
            // Check runtime name for equality
            if (_runtime_name != runtime->runtime_name()) {
                return false;
            }
            // Check type name for equality
            return _type_name.empty() || _type_name == runtime->type_name();
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(Type const& other) const
        {
            // Check that the other Runtime is specialized
            auto type = boost::get<basic_runtime<Type>>(&other);
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
        boost::optional<object_type> _object;
    };

    /**
     * Stream insertion operator for Runtime type.
     * @tparam Type The type of a runtime type.
     * @param os The output stream to write the type to.
     * @param type The Runtime type to write.
     * @return Returns the given output stream.
     */
    template <typename Type>
    std::ostream& operator<<(std::ostream& os, basic_runtime<Type> const& type)
    {
        os << basic_runtime<Type>::name();
        if (type.runtime_name().empty()) {
            return os;
        }
        os << "['" << type.runtime_name();
        if (!type.type_name().empty()) {
            os << "', '" << type.type_name();
        }
        os << "']";
        return os;
    }

    /**
     * Equality operator for runtime.
     * @tparam Type The type of a runtime type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    template <typename Type>
    bool operator==(basic_runtime<Type> const& left, basic_runtime<Type> const& right)
    {
        return left.runtime_name() == right.runtime_name() && left.type_name() == right.type_name();
    }

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Runtime type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct hash<puppet::runtime::types::basic_runtime<Type>>
    {
        /**
         * Hashes the Runtime type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::basic_runtime<Type> const& type) const
        {
            static const size_t name_hash = boost::hash_value(puppet::runtime::types::basic_runtime<Type>::name());

            size_t seed = 0;
            hash_combine(seed, name_hash);
            hash_combine(seed, type.runtime_name());
            hash_combine(seed, type.type_name());
            if (type.object()) {
                hash_combine(seed, *type.object());
            }
            return seed;
        }
    };
}
