/**
 * @file
 * Declares the runtime scope.
 */
#pragma once

#include "values/value.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <cstdint>

namespace puppet { namespace runtime {

    /**
     * Represents an assigned variable.
     */
    struct assigned_variable
    {
        /**
         * Constructs an assigned variable with the given value and location.
         * @param value The value of the variable.
         * @param path The path of the file where the variable was assigned.
         * @param line The line where the variable was assigned.
         */
        assigned_variable(std::shared_ptr<values::value const> value, std::shared_ptr<std::string> path, size_t line);

        /**
         * Gets the value of the variable.
         * @return Returns the value of the variable.
         */
        std::shared_ptr<values::value const> const& value() const;

        /**
         * Gets the path of the file where the variable was assigned.
         * @return Returns the path of the file where the variable was assigned.
         */
        std::string const* path() const;

        /**
         * Gets the line where the variable was assigned.
         * @return Returns the line where the variable was assigned.
         */
        size_t line() const;

     private:
        std::shared_ptr<values::value const> _value;
        std::shared_ptr<std::string> _path;
        size_t _line;
    };

    /**
     * Represents a runtime scope.
     */
    struct scope
    {
        /**
         * Constructs a scope.
         * @param parent The parent scope.
         * @param name The name of the scope (e.g. foo).
         * @param display_name The display name of the scope (e.g. Class[foo]).
         */
        explicit scope(std::shared_ptr<scope> parent = nullptr, std::string name = std::string(), std::string display_name = std::string());

        /**
         * Gets the name of the scope.
         * @return Returns the name of scope.
         */
        std::string const& name() const;

        /**
         * Gets the display name of the scope.
         * @return Returns the display name of scope.
         */
        std::string const& display_name() const;

        /**
         * Gets the parent scope.
         * @return Returns the parent scope or nullptr if at top scope.
         */
        std::shared_ptr<scope> const& parent() const;

        /**
         * Qualifies the given name using the scope's name.
         * @param name The name to qualify.
         * @return Returns the fully-qualified name.
         */
        std::string qualify(std::string const& name) const;

        /**
         * Sets a variable in the scope.
         * @param name The name of the variable.
         * @param value The value of the variable.
         * @param path The path of the file where the variable is being assigned or nullptr if unknown.
         * @param line The line number where the variable is being assigned or 0 if unknown.
         * @return Returns a pointer to the assigned variable or nullptr if the variable already exists in the scope.
         */
        assigned_variable const* set(std::string name, std::shared_ptr<values::value const> value, std::shared_ptr<std::string> path = nullptr, size_t line = 0);

        /**
         * Gets a variable in the scope.
         * @param name The name of the variable to get.
         * @return Returns the assigned variable or nullptr if the variable does not exist in the scope.
         */
        assigned_variable const* get(std::string const& name) const;

     private:
        std::shared_ptr<scope> _parent;
        std::string _name;
        std::string _display_name;
        std::unordered_map<std::string, assigned_variable> _variables;
    };

    /**
     * Stream insertion operator for runtime scope.
     * @param os The output stream to write the runtime scope to.
     * @param s The runtime scope to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, scope const& s);

}}  // puppet::runtime
