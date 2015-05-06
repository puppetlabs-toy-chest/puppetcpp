/**
 * @file
 * Declares the hash type.
 */
#pragma once

#include "scalar.hpp"
#include "data.hpp"
#include "struct.hpp"
#include "string.hpp"
#include "../values/hash.hpp"
#include <boost/functional/hash.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Hash type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct basic_hash
    {
        /**
         * Constructs a Hash type.
         * @param key_type The key type of the hash.  Defaults to the Scalar type.
         * @param element_type The element type of the hash.  Defaults to the Data type.
         * @param from The "from" type parameter.
         * @param to The "to" type parameter.
         */
        explicit basic_hash(Type key_type = scalar(), Type element_type = data(), int64_t from = std::numeric_limits<int64_t>::min(), int64_t to = std::numeric_limits<int64_t>::max()) :
            _key_type(std::move(key_type)),
            _element_type(std::move(element_type)),
            _from(from),
            _to(to)
        {
        }

        /**
         * Gets the key type of the hash.
         * @return Returns the element type of the hash.
         */
        Type const& key_type() const
        {
            return _key_type;
        }

        /**
         * Gets the element type of the hash.
         * @return Returns the element type of the hash.
         */
        Type const& element_type() const
        {
            return _element_type;
        }

        /**
        * Gets the "from" type parameter.
        * @return Returns the "from" type parameter.
        */
        int64_t from() const
        {
            return _from;
        }

        /**
         * Gets the "to" type parameter.
         * @return Returns the "to" type parameter.
         */
        int64_t to() const
        {
            return _to;
        }

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Hash).
         */
        static const char* name()
        {
            return "Hash";
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

            // Check for size is range
            int64_t size = static_cast<int64_t>(ptr->size());
            if (!(_to < _from ? (size >= _to && size <= _from) : (size >= _from && size <= _to))) {
                return false;
            }

            // Check that each key and element is of the appropriate types
            for (auto const& kvp : *ptr) {
                if (!is_instance(kvp.first, _key_type) || !is_instance(kvp.second, _element_type)) {
                    return false;
                }
            }
            return true;
        }

        /**
         * Determines if the given type is a specialization (i.e. more specific) of this type.
         * @param other The other type to check for specialization.
         * @return Returns true if the other type is a specialization or false if not.
         */
        bool is_specialization(Type const& other) const
        {
            // For the other type to be a specialization, it must be an Hash or Struct
            // The key types must match
            // The element types must match
            // And the range of other needs to be inside of this type's range
            int64_t from, to;
            auto hash = boost::get<basic_hash<Type>>(&other);
            if (hash) {
                // Check for Hash
                if (hash->key_type() != _key_type || hash->element_type() != _element_type) {
                    return false;
                }
                from = hash->from();
                to = hash->to();
            } else {
                // Check for a Struct
                auto structure = boost::get<basic_struct<Type>>(&other);
                if (!structure || !boost::get<string>(&_key_type)) {
                    return false;
                }
                // Ensure all elements of the structure are of the element type
                for (auto& kvp : structure->types()) {
                    if (kvp.second != _element_type) {
                        return false;
                    }
                }
                from = to = structure->types().size();
            }
            // Check for equality
            if (from == _from && to == _to) {
                return false;
            }
            return std::min(from, to) >= std::min(_from, _to) &&
                   std::max(from, to) <= std::max(_from, _to);
        }

     private:
        Type _key_type;
        Type _element_type;
        int64_t _from;
        int64_t _to;
    };

    /**
     * Stream insertion operator for hash type.
     * @tparam Type The "runtime type" type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    template <typename Type>
    std::ostream& operator<<(std::ostream& os, basic_hash<Type> const& type)
    {
        os << basic_hash<Type>::name() << '[' << type.key_type() << ", " << type.element_type();
        bool from_default = type.from() == std::numeric_limits<int64_t>::min();
        bool to_default = type.to() == std::numeric_limits<int64_t>::max();
        if (from_default && to_default) {
            // Only output the types
            os << ']';
            return os;
        }
        os << ", ";
        if (from_default) {
            os << "default";
        } else {
            os << type.from();
        }
        os << ", ";
        if (to_default) {
            os << "default";
        } else {
            os << type.to();
        }
        os << ']';
        return os;
    }

    /**
     * Equality operator for hash.
     * @tparam Type The "runtime type" type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    template <typename Type>
    bool operator==(basic_hash<Type> const& left, basic_hash<Type> const& right)
    {
        return left.from() == right.from() &&
               left.to() == right.to() &&
               left.key_type() == right.key_type() &&
               left.element_type() == right.element_type();
    }

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Hash type.
     * @tparam Type The "runtime type" type.
     */
    template <typename Type>
    struct hash<puppet::runtime::types::basic_hash<Type>>
    {
        /**
         * Hashes the Hash type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::basic_hash<Type> const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::basic_hash<Type>::name());
            hash_combine(seed, type.key_type());
            hash_combine(seed, type.element_type());
            hash_combine(seed, type.from());
            hash_combine(seed, type.to());
            return seed;
        }
    };
}
