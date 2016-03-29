/**
 * @file
 * Declares the callable type.
 */
#pragma once

#include "../values/forward.hpp"
#include "../../compiler/ast/ast.hpp"
#include <boost/optional.hpp>
#include <ostream>
#include <vector>
#include <limits>
#include <unordered_map>

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    // Forward declaration of call context
    struct call_context;

}}}}  // namespace puppet::compiler::evaluation::functions

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of alias
    struct alias;

    /**
     * Represents the Puppet Callable type.
     */
    struct callable
    {
        /**
         * Constructs a Callable type.
         * If neither min or max are specified, the arguments to the Callable must match the types.
         * If min < types.size(), types with an index >= min are optional.
         * If max > types.size(), the last type repeats until the max.
         * If no types and no min/max are given, the Callable describes anything "callable" (i.e. Callable[0, default]).
         * A Callable[0, 0] accepts no arguments.
         * If no types are given and the min or max count are not 0, then the Callable describes the untyped arity and does not constrain the parameter types.
         * @param types The parameter types of the callable.
         * @param min The minimum number of parameters.
         * @param max The maximum number of parameters.
         * @param block_type The Callable representing a block parameter.
         */
        explicit callable(
            std::vector<std::unique_ptr<values::type>> types = {},
            int64_t min = 0,
            int64_t max = std::numeric_limits<int64_t>::max(),
            std::unique_ptr<values::type> block_type = nullptr
         );

        /**
         * Copy constructor for callable type.
         * @param other The other callable type to copy from.
         */
        callable(callable const& other);

        /**
         * Move constructor for callable type.
         * Uses the default implementation.
         */
        callable(callable&&) noexcept = default;

        /**
         * Copy assignment operator for callable type.
         * @param other The other callable type to copy assign from.
         * @return Returns this callable type.
         */
        callable& operator=(callable const& other);

        /**
         * Move assignment operator for callable type.
         * Uses the default implementation.
         * @return Returns this callable type.
         */
        callable& operator=(callable&&) noexcept = default;

        /**
         * Gets the parameter types for the callable.
         * @return Returns the parameter types for the callable.
         */
        std::vector<std::unique_ptr<values::type>> const& types() const;

        /**
         * Gets the minimum number of parameters for the callable.
         * @return Returns the minimum number of parameters for the callable.
         */
        int64_t min() const;

        /**
         * Gets the maximum number of parameters for the callable.
         * @return Returns the maximum number of parameters for the callable.
         */
        int64_t max() const;

        /**
         * Gets the block type parameter for the callable.
         * @return Returns the block type parameter for the callable.
         */
        std::unique_ptr<values::type> const& block_type() const;

        /**
         * Gets the underlying block signature for the callable (i.e. dereferences a block signature of Optional[Callable[...]]).
         * @return Returns the pairing of block signature and a boolean signifying whether or not the block is required; if no block is accepted, (nullptr, false) is returned.
         */
        std::pair<callable const*, bool> block() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type (i.e. Callable).
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
         * Determines if the type is real (i.e. actual type vs. an alias/variant that never resolves to an actual type).
         * @param map The map to keep track of encountered type aliases.
         * @return Returns true if the type is real or false if it never resolves to an actual type.
         */
        bool is_real(std::unordered_map<values::type const*, bool>& map) const;

        /**
         * Writes a representation of the type to the given stream.
         * @param stream The stream to write to.
         * @param expand True to specify that type aliases should be expanded or false if not.
         */
        void write(std::ostream& stream, bool expand = true) const;

        /**
         * Determines if a function call can be dispatched to a function matching this Callable's signature.
         * @param context The call context to determine if the call can be dispatched.
         * @return Returns true if the call can be dispatched or false if not.
         */
        bool can_dispatch(compiler::evaluation::functions::call_context const& context) const;

        /**
         * Finds the first argument with a parameter type mismatch.
         * @param arguments The arguments to find a parameter type mismatch for.
         * @return Returns the index of the argument if a mismatch is found or less than 0 if all arguments conform to the parameter types.
         */
        int64_t find_mismatch(values::array const& arguments) const;

        /**
         * Gets the type of the parameter at the given index.
         * @param index The index of the parameter.
         * @return Returns the type of the parameter at the given index or nullptr for an invalid index.
         */
        values::type const* parameter_type(int64_t index) const;

     private:
        std::vector<std::unique_ptr<values::type>> _types;
        int64_t _min;
        int64_t _max;
        std::unique_ptr<values::type> _block_type;
    };

    /**
     * Stream insertion operator for callable type.
     * @param os The output stream to write the type to.
     * @param type The callable type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, callable const& type);

    /**
     * Equality operator for callable.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are equal or false if not.
     */
    bool operator==(callable const& left, callable const& right);

    /**
     * Inequality operator for callable.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(callable const& left, callable const& right);

    /**
     * Hashes the callable type.
     * @param type The callable type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(callable const& type);

}}}  // namespace puppet::runtime::types
