/**
 * @file
 * Declares the tuple type.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <vector>
#include <cstdint>
#include <limits>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of recursion_guard
    struct recursion_guard;

    /**
     * Represents the Puppet Tuple type.
     */
    struct tuple
    {
        /**
         * Constructs a Tuple type.
         * @param types The types that make up the tuple.
         * @param from The "from" type parameter.
         * @param to The "to" type parameter.
         */
        explicit tuple(
            std::vector<std::unique_ptr<values::type>> types = std::vector<std::unique_ptr<values::type>>(),
            int64_t from = 0,
            int64_t to = std::numeric_limits<int64_t>::max());

        /**
         * Copy constructor for tuple type.
         * @param other The other tuple type to copy from.
         */
        tuple(tuple const& other);

        /**
         * Move constructor for tuple type.
         * Uses the default implementation.
         */
        tuple(tuple&&) noexcept = default;

        /**
         * Copy assignment operator for tuple type.
         * @param other The other tuple type to copy assign from.
         * @return Returns this tuple type.
         */
        tuple& operator=(tuple const& other);

        /**
         * Move assignment operator for tuple type.
         * Uses the default implementation.
         * @return Returns this tuple type.
         */
        tuple& operator=(tuple&&) noexcept = default;

        /**
         * Gets the tuple types.
         * @return Returns the tuple types.
         */
        std::vector<std::unique_ptr<values::type>> const& types() const;

        /**
         * Gets the "from" type parameter.
         * @return Returns the "from" type parameter.
         */
        int64_t from() const;

        /**
         * Gets the "to" type parameter.
         * @return Returns the "to" type parameter.
         */
        int64_t to() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Tuple).
         */
        static char const* name();

        /**
         * Determines if the given value is an instance of this type.
         * @param value The value to determine if it is an instance of this type.
         * @param guard The recursion guard to use for aliases.
         * @return Returns true if the given value is an instance of this type or false if not.
         */
        bool is_instance(values::value const& value, recursion_guard& guard) const;

        /**
         * Determines if the given type is assignable to this type.
         * @param other The other type to check for assignability.
         * @param guard The recursion guard to use for aliases.
         * @return Returns true if the given type is assignable to this type or false if the given type is not assignable to this type.
         */
        bool is_assignable(values::type const& other, recursion_guard& guard) const;

        /**
         * Writes a representation of the type to the given stream.
         * @param stream The stream to write to.
         * @param expand True to specify that type aliases should be expanded or false if not.
         */
        void write(std::ostream& stream, bool expand = true) const;

        /**
         * Stores a default shared instance used internally by other Puppet types.
         */
        static tuple const instance;

     private:
        std::vector<std::unique_ptr<values::type>> _types;
        int64_t _from;
        int64_t _to;
    };

    /**
     * Stream insertion operator for tuple type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, tuple const& type);

    /**
     * Equality operator for tuple.
     * @tparam Type The type of a runtime type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(tuple const& left, tuple const& right);

    /**
     * Inequality operator for tuple.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(tuple const& left, tuple const& right);

    /**
     * Hashes the tuple type.
     * @param type The tuple type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(tuple const& type);

}}}  // namespace puppet::runtime::types
