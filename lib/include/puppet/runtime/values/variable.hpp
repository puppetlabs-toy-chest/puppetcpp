/**
 * @file
 * Declares the variable runtime value.
 */
#pragma once

#include "../../cast.hpp"
#include <boost/functional/hash.hpp>
#include <ostream>
#include <memory>

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents a reference to a variable.
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
         * @param value The variable's value.
         */
        basic_variable(std::string name, std::shared_ptr<value_type const> value) :
            _name(rvalue_cast(name)),
            _value(rvalue_cast(value))
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
         * Gets the shared pointer to the variable's value.
         * @return Returns the shared pointer to the variable's value.
         */
        std::shared_ptr<value_type const> const& value_ptr() const
        {
            return _value;
        }

        /**
         * Assigns the given value to the variable.
         * @param value The new value of the variable.
         */
        void assign(std::shared_ptr<value_type const> const& value)
        {
            _value = value;
        }

    private:
        std::string _name;
        std::shared_ptr<value_type const> _value;
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
