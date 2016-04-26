/**
 * @file
 * Declares the variable runtime value.
 */
#pragma once

#include "forward.hpp"
#include <ostream>
#include <memory>

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents a reference to a variable.
     */
    struct variable
    {
        /**
         * Constructs a variable reference.
         * @param name The name of the variable.
         * @param value The variable's value.
         */
        variable(std::string name, std::shared_ptr<values::value const> value);

        /**
         * Gets the name of the variable.
         * @return Returns the name of the variable.
         */
        std::string const& name() const;

        /**
         * Gets the value of the variable.
         * @return Returns the value of the variable.
         */
        values::value const& value() const;

        /**
         * Gets the shared pointer to the variable's value.
         * @return Returns the shared pointer to the variable's value.
         */
        std::shared_ptr<values::value const> const& shared_value() const;

        /**
         * Assigns the given value to the variable.
         * @param value The new value of the variable.
         */
        void assign(std::shared_ptr<values::value const> value);

    private:
        std::string _name;
        std::shared_ptr<values::value const> _value;
    };

    /**
     * Stream insertion operator for runtime variable.
     * @param os The output stream to write the runtime variable to.
     * @param variable The runtime variable to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, values::variable const& variable);

    /**
     * Equality operator for variable.
     * @param left The left variable to compare.
     * @param right The right variable to compare.
     * @return Returns true if the values referenced by the variables are equal or false if not; does not compare variable names.
     */
    bool operator==(variable const& left, variable const& right);

    /**
     * Inequality operator for variable.
     * @param left The left variable to compare.
     * @param right The right variable to compare.
     * @return Returns true if variables are not equal or false if they are equal.
     */
    bool operator!=(variable const& left, variable const& right);

    /**
     * Hashes the variable value.
     * @param variable The variable value to hash.
     * @return Returns the hash value for the value.
     */
    size_t hash_value(values::variable const& variable);

}}}  // namespace puppet::runtime::values
