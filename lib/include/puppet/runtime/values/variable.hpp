/**
 * @file
 * Declares the variable runtime value.
 */
#pragma once

#include "../../cast.hpp"
#include <boost/functional/hash.hpp>
#include <ostream>

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents a reference to a variable.
     * Having this as a runtime value prevents unnecessary copying of a variable's value.
     * Thus, '$a = $b' simply points $a's value at what $b was set to.
     * @tparam Value The runtime value type.
     */
    template <typename Value>
    struct basic_variable
    {
        /**
         * The runtime value type.
         */
        typedef Value value_type;

        /**
         * Constructs a variable reference.
         * @param name The name of the variable.
         * @param val The current value of the variable.
         * @param match The variable is a match variable.
         */
        basic_variable(std::string name, value_type const* val, bool match) :
            _name(rvalue_cast(name)),
            _value(val),
            _match(match)
        {
        }

        /**
         * Gets the name of the variable.
         * @return Returns the name of the variable.
         */
        std::string const& name() const
        {
            return _name;
        }

        /**
         * Gets the value of the variable.
         * @return Returns the value of the variable.
         */
        value_type const& value() const
        {
            static value_type undefined;
            return _value ? *_value : undefined;
        }

        /**
         * Determines if the variable is a match variable.
         * @return Returns true if the variable is a match variable or false if not.
         */
        bool match() const
        {
            return _match;
        }

        /**
         * Updates the value of the variable.
         * @param ptr The pointer to the variable's value.
         */
        void update(value_type const* ptr)
        {
            _value = ptr;
        }

    private:
        std::string _name;
        value_type const* _value;
        bool _match;
    };

    /**
     * Equality operator for variable.
     * @tparam Value The type of runtime value.
     * @param left The left variable to compare.
     * @param right The right variable to compare.
     * @return Returns true if the values referenced by the variables are equal or false if not; does not compare variable names.
     */
    template <typename Value>
    bool operator==(basic_variable<Value> const& left, basic_variable<Value> const& right)
    {
        // Optimization: if both variables point to the same value, they are equal
        if (&left.value() == &right.value()) {
            return true;
        }
        return equals(left.value(), right.value());
    }

    /**
     * Stream insertion operator for runtime variable.
     * @tparam Value The type of runtime value.
     * @param os The output stream to write the runtime variable to.
     * @param variable The runtime variable to write.
     * @return Returns the given output stream.
     */
    template <typename Value>
    std::ostream& operator<<(std::ostream& os, basic_variable<Value> const& variable)
    {
        os << variable.value();
        return os;
    }

}}}  // puppet::runtime::values

namespace boost {
    /**
     * Hash specialization for variable.
     * @tparam Value The type of runtime value.
     */
    template <typename Value>
    struct hash<puppet::runtime::values::basic_variable<Value>>
    {
        /**
         * Hashes the variable.
         * @param variable The variable to hash.
         * @return Returns the hash value for the variable.
         */
        size_t operator()(puppet::runtime::values::basic_variable<Value> const& variable) const
        {
            return hash_value(variable.value());
        }
    };
}
