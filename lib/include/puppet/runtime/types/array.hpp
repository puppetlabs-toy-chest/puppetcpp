/**
 * @file
 * Declares the array type.
 */
#pragma once

#include "tuple.hpp"
#include "data.hpp"
#include "../values/array.hpp"
#include "../../cast.hpp"
#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <limits>
#include <ostream>
#include <functional>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Array type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct basic_array
    {
        /**
         * Constructs an Array type.
         * @param type The element type of the array.  Defaults to the Data type.
         * @param from The "from" type parameter.
         * @param to The "to" type parameter.
         */
        explicit basic_array(Type type = data(), int64_t from = std::numeric_limits<int64_t>::min(), int64_t to = std::numeric_limits<int64_t>::max()) :
            _element_type(rvalue_cast(type)),
            _from(from),
            _to(to)
        {
        }

        /**
         * Gets the element type of the array.
         * @return Returns the element type of the array.
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
         * @return Returns the name of the type (i.e. Array).
         */
        static const char* name()
        {
            return "Array";
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

            // Check for array
            auto ptr = boost::get<values::basic_array<Value>>(&value);
            if (!ptr) {
                return false;
            }

            // Check for size is range
            int64_t size = static_cast<int64_t>(ptr->size());
            if (!(_to < _from ? (size >= _to && size <= _from) : (size >= _from && size <= _to))) {
                return false;
            }

            // Check that each element is of the type
            for (auto const& element : *ptr) {
                if (!unsafe_is_instance(&element, &_element_type)) {
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
            // For the other type to be a specialization, it must be an Array or Tuple
            // For Tuple, the number of types must be 1
            // The element types must match
            // And the range of other needs to be inside of this type's range
            int64_t from, to;
            auto array = boost::get<basic_array<Type>>(&other);
            if (!array) {
                // Check for Array[ElementType]
                if (array->element_type() != _element_type) {
                    return false;
                }
                from = array->from();
                to = array->to();
            } else {
                // Check for a Tuple[ElementType]
                auto tuple = boost::get<basic_tuple<Type>>(&other);
                if (!tuple || tuple->types().size() != 1 || tuple->types().front() != _element_type) {
                    return false;
                }
                from = tuple->from();
                to = tuple->to();
            }
            // Check for equality
            if (from == _from && to == _to) {
                return false;
            }
            return std::min(from, to) >= std::min(_from, _to) &&
                   std::max(from, to) <= std::max(_from, _to);
        }

     private:
        Type _element_type;
        int64_t _from;
        int64_t _to;
    };

    /**
     * Stream insertion operator for array type.
     * @tparam Type The runtime "type" type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    template <typename Type>
    std::ostream& operator<<(std::ostream& os, basic_array<Type> const& type)
    {
        os << basic_array<Type>::name() << '[' << type.element_type();
        bool from_default = type.from() == std::numeric_limits<int64_t>::min();
        bool to_default = type.to() == std::numeric_limits<int64_t>::max();
        if (from_default && to_default) {
            // Only output the type
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
     * Equality operator for array.
     * @tparam Type The "runtime type" type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    template <typename Type>
    bool operator==(basic_array<Type> const& left, basic_array<Type> const& right)
    {
        return left.from() == right.from() && left.to() == right.to() && left.element_type() == right.element_type();
    }

}}}  // puppet::runtime::types

namespace boost {
    /**
     * Hash specialization for Array type.
     * @tparam Type The "runtime type" type.
     */
    template <typename Type>
    struct hash<puppet::runtime::types::basic_array<Type>>
    {
        /**
         * Hashes the Array type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::basic_array<Type> const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::basic_array<Type>::name());
            hash_combine(seed, type.element_type());
            hash_combine(seed, type.from());
            hash_combine(seed, type.to());
            return seed;
        }
    };
}
