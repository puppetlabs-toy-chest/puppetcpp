/**
 * @file
 * Declares the struct type.
 */
#pragma once

#include "../values/hash.hpp"
#include "../values/undef.hpp"
#include <boost/functional/hash.hpp>
#include <ostream>
#include <unordered_map>
#include <string>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Struct type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct basic_struct
    {
        /**
         * Constructs a Struct type.
         * @param types The types that make up the struct.
         */
        explicit basic_struct(std::unordered_map<std::string, Type> types = std::unordered_map<std::string, Type>()) :
            _types(std::move(types))
        {
        }

        /**
         * Gets the struct types.
         * @return Returns the struct types.
         */
        std::unordered_map<std::string, Type> const& types() const
        {
            return _types;
        }

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Struct).
         */
        static const char* name()
        {
            return "Struct";
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
            // Forward declaration of is_instance for recursion
            bool is_instance(Value const&, Type const&);

            // Check for hash
            auto ptr = boost::get<values::basic_hash<Value>>(&value);
            if (!ptr) {
                return false;
            }

            // If no types, only empty hashes match
            if (_types.empty()) {
                return ptr->empty();
            }

            // Go through each type and ensure it's in the hash
            size_t count = 0;
            for (auto const& kvp : _types) {
                auto it = ptr->find(kvp.first);
                if (it == ptr->end()) {
                    if (!is_instance(values::undef(), kvp.second)) {
                        return false;
                    }
                    continue;
                }

                if (!is_instance(it->second, kvp.second)) {
                    return false;
                }

                ++count;
            }
            // Ensure that the hash doesn't contain more keys than what is present in the types
            return count == ptr->size();
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(Type const& other) const
        {
            // Check for another Struct
            auto ptr = boost::get<basic_struct<Type>>(&other);
            if (!ptr) {
                return false;
            }

            // Check for less types (i.e. this type is more specialized)
            auto& other_types = ptr->types();
            if (other_types.size() < _types.size()) {
                return false;
            }
            // All values must match
            for (auto const& kvp : _types) {
                auto other_kvp = other_types.find(kvp.first);
                if (other_kvp == other_types.end()) {
                    return false;
                }
                if (kvp.second != other_kvp->second) {
                    return false;
                }
            }
            // If the other type has more types, it is more specialized
            if (other_types.size() > _types.size()) {
                return true;
            }
            return false;
        }

     private:
        std::unordered_map<std::string, Type> _types;
    };

    /**
     * Stream insertion operator for struct type.
     * @tparam Type The type of a runtime type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    template <typename Type>
    std::ostream& operator<<(std::ostream& os, basic_struct<Type> const& type)
    {
        os << basic_struct<Type>::name();
        if (type.types().empty()) {
            return os;
        }
        os << "[{";
        bool first = true;
        for (auto const& kvp : type.types()) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << kvp.first << " => " << kvp.second;
        }
        os << "}]";
        return os;
    }

    /**
     * Equality operator for struct.
     * @tparam Type The type of a runtime type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    template <typename Type>
    bool operator==(basic_struct<Type> const& left, basic_struct<Type> const& right)
    {
        // Check the types
        auto const& left_types = left.types();
        auto const& right_types = right.types();
        if (left_types.size() != right_types.size()) {
            return false;
        }

        // Ensure all types match
        for (auto const& left_kvp : left_types) {
            auto right_kvp = right_types.find(left_kvp.first);
            if (right_kvp == right_types.end()) {
                return false;
            }
            if (left_kvp.second != right_kvp->second) {
                return false;
            }
        }
        return true;
    }

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Struct type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct hash<puppet::runtime::types::basic_struct<Type>>
    {
        /**
         * Hashes the Struct type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::basic_struct<Type> const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::basic_struct<Type>::name());
            hash_range(seed, type.types().begin(), type.types().end());
            return seed;
        }
    };
}
