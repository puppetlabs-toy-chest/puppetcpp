/**
 * @file
 * Declares the variant type.
 */
#pragma once

#include "../../cast.hpp"
#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <ostream>
#include <vector>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Variant type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct basic_variant
    {
        /**
         * Constructs a Variant type.
         * @param types The types that make up the variant.
         */
        explicit basic_variant(std::vector<Type> types = std::vector<Type>()) :
            _types(rvalue_cast(types))
        {
        }

        /**
         * Gets the variant types.
         * @return Returns the variant types.
         */
        std::vector<Type> const& types() const
        {
            return _types;
        }

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Variant).
         */
        static const char* name()
        {
            return "Variant";
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
            // Forward declaration of unsafe_is_instance for recursion
            bool unsafe_is_instance(void const*, void const*);

            // Go through each type and ensure one matches
            for (auto const& type : _types) {
                if (unsafe_is_instance(&value, &type)) {
                    return true;
                }
            }
            return false;
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(Type const& other) const
        {
            // Check for another Variant
            auto ptr = boost::get<basic_variant<Type>>(&other);
            if (!ptr) {
                return false;
            }

            // Check for less types (i.e. this type is more specialized)
            auto& other_types = ptr->types();
            if (other_types.size() < _types.size()) {
                return false;
            }
            // All types must match
            for (size_t i = 0; i < _types.size(); ++i) {
                if (_types[i] != other_types[i]) {
                    return false;
                }
            }
            // If the other type has more types, it is more specialized
            return other_types.size() > _types.size();
        }

    private:
        std::vector<Type> _types;
    };

    /**
     * Stream insertion operator for variant type.
     * @tparam Type The type of a runtime type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    template <typename Type>
    std::ostream& operator<<(std::ostream& os, basic_variant<Type> const& type)
    {
        os << basic_variant<Type>::name();
        if (type.types().empty()) {
            return os;
        }
        os << '[';
        bool first = true;
        for (auto const& element : type.types()) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << element;
        }
        os << ']';
        return os;
    }

    /**
     * Equality operator for variant.
     * @tparam Type The type of a runtime type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    template <typename Type>
    bool operator==(basic_variant<Type> const& left, basic_variant<Type> const& right)
    {
        // Check the number of types
        auto const& left_types = left.types();
        auto const& right_types = right.types();
        if (left_types.size() != right_types.size()) {
            return false;
        }

        // Ensure all types are equal
        for (size_t i = 0; i < left_types.size(); ++i) {
            if (left_types[i] != right_types[i]) {
                return false;
            }
        }
        return true;
    }

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Variant type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct hash<puppet::runtime::types::basic_variant<Type>>
    {
        /**
         * Hashes the Variant type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::basic_variant<Type> const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::basic_variant<Type>::name());
            hash_range(seed, type.types().begin(), type.types().end());
            return seed;
        }
    };
}
