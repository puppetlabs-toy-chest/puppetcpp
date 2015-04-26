/**
 * @file
 * Declares the tuple type.
 */
#pragma once

#include "../values/array.hpp"
#include <boost/functional/hash.hpp>
#include <ostream>
#include <vector>
#include <cstdint>
#include <limits>

namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Puppet Tuple type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct basic_tuple
    {
        /**
         * Constructs a Tuple type.
         * @param types The types that make up the tuple.
         * @param from The "from" type parameter.
         * @param to The "to" type parameter.
         */
        explicit basic_tuple(std::vector<Type> types = std::vector<Type>(), int64_t from = std::numeric_limits<int64_t>::min(), int64_t to = std::numeric_limits<int64_t>::max()) :
            _types(std::move(types)),
            _from(from),
            _to(to)
        {
        }

        /**
         * Gets the tuple types.
         * @return Returns the tuple types.
         */
        std::vector<Type> const& types() const
        {
            return _types;
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
         * @return Returns the name of the type (i.e. Tuple).
         */
        static const char* name()
        {
            return "Tuple";
        }

        /**
         * Determines if the given value is an instance of this type.
         * @tparam Value The type of the runtime value.
         * @param value The value to determine if it is an instance of this type.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        template <typename Value>
        bool is_instance(Value const& value) const
        {
            // Forward declaration of is_instance for recursion
            bool is_instance(Value const&, Type const&);

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

            // If no types, only empty arrays match
            if (_types.empty()) {
                return size == 0;
            }

            // For each element, check that its type is in the tuple
            for (size_t i = 0; i < ptr->size(); ++i) {
                auto const& element = (*ptr)[i];

                // If this element's position is in the tuple, match the type
                if (i < _types.size()) {
                    if (!is_instance(element, _types[i])) {
                        return false;
                    }
                } else if (!is_instance(element, _types.back())) {
                    // Otherwise, it didn't match the last element in the tuple
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
            // Check for another Tuple
            auto ptr = boost::get<basic_tuple<Type>>(&other);
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
            if (other_types.size() > _types.size()) {
                return true;
            }
            // Check for equality
            if (ptr->from() == _from && ptr->to() == _to) {
                return false;
            }
            return std::min(ptr->from(), ptr->to()) >= std::min(_from, _to) &&
                   std::max(ptr->from(), ptr->to()) <= std::max(_from, _to);
        }

     private:
        std::vector<Type> _types;
        int64_t _from;
        int64_t _to;
    };

    /**
     * Stream insertion operator for tuple type.
     * @tparam Type The type of a runtime type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    template <typename Type>
    std::ostream& operator<<(std::ostream& os, basic_tuple<Type> const& type)
    {
        os << basic_tuple<Type>::name();
        if (type.types().empty()) {
            return os;
        }
        os << '[';
        bool first = true;
        for (auto const& type : type.types()) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << type;
        }
        // If the from, to, and size of the types are equal, only output the types
        int64_t size = static_cast<int64_t>(type.types().size());
        if (type.from() == size && type.to() == size) {
            os << ']';
            return os;
        }
        os << ", ";
        if (type.from() == std::numeric_limits<int64_t>::min()) {
            os << "default";
        } else {
            os << type.from();
        }
        os << ", ";
        if (type.to() == std::numeric_limits<int64_t>::max()) {
            os << "default";
        } else {
            os << type.to();
        }
        os << ']';
        return os;
    }

    /**
     * Equality operator for tuple.
     * @tparam Type The type of a runtime type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    template <typename Type>
    bool operator==(basic_tuple<Type> const& left, basic_tuple<Type> const& right)
    {
        // Check the range first
        if (left.from() != right.from() || left.to() != right.to()) {
            return false;
        }

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
     * Hash specialization for Tuple type.
     * @tparam Type The type of a runtime type.
     */
    template <typename Type>
    struct hash<puppet::runtime::types::basic_tuple<Type>>
    {
        /**
         * Hashes the Tuple type.
         * @param type The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::types::basic_tuple<Type> const& type) const
        {
            size_t seed = 0;
            hash_combine(seed, puppet::runtime::types::basic_tuple<Type>::name());
            hash_range(seed, type.types().begin(), type.types().end());
            hash_combine(seed, type.from());
            hash_combine(seed, type.to());
            return seed;
        }
    };
}
