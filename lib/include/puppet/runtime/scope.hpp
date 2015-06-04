/**
 * @file
 * Declares the runtime scope.
 */
#pragma once

#include "values/value.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <tuple>
#include <cstdint>
#include <regex>
#include <deque>

namespace puppet { namespace runtime {

    /**
     * Represents a runtime scope.
     */
    struct scope
    {
        /**
         * Constructs a scope.
         * @param name The name of the scope (e.g. foo).
         * @param display_name The display name of the scope (e.g. Class[Foo]).
         * @param parent The parent scope.
         */
        explicit scope(std::string name, std::string display_name, scope* parent = nullptr);

        /**
         * Constructs an ephemeral scope.
         */
        explicit scope(scope* parent);

        /**
         * Determines if the scope is ephemeral.
         * @return Returns true if the scope is ephemeral or false if not.
         */
        bool ephemeral() const;

        /**
         * Gets the name of the scope.
         * Note: for ephemeral scopes, this returns the name of the parent scope.
         * @return Returns the name of scope.
         */
        std::string const& name() const;

        /**
         * Gets the display name of the scope.
         * Note: for ephemeral scopes, this returns the display name of the parent scope.
         * @return Returns the display name of scope.
         */
        std::string const& display_name() const;

        /**
         * Qualifies the given name using the scope's name.
         * @param name The name to qualify.
         * @return Returns the fully-qualified name.
         */
        std::string qualify(std::string const& name) const;

        /**
         * Sets a variable in the scope.
         * @param name The name of the variable.
         * @param val The value of the variable.
         * @param line The line number where the variable is being assigned or 0 if unknown.
         * @return Returns a pointer to the value that was set or nullptr if the value already exists in this scope.
         */
        values::value const* set(std::string name, values::value val, size_t line = 0);

        /**
         * Gets a variable in the scope.
         * @param name The name of the variable to get.
         * @return Returns the value of the variable or nullptr if not found.
         */
        values::value const* get(std::string const& name) const;

        /**
         * Gets the line where the variable was assigned.
         * @param name The variable name.
         * @return Returns the line where the variable was assigned or 0 if unknown.
         */
        size_t where(std::string const& name);

        /**
         * Gets the parent scope.
         * @return Returns the parent scope or nullptr if at top scope.
         */
        scope const* parent() const;

        /**
         * Sets the given matches into the scope.
         * This will set the $0 - $n variables.
         * @param matches The matches to set.
         */
        void set(std::smatch const& matches);

        /**
         * Gets a match variable by index.
         * @param index The index of the match variable.
         * @return Returns the match variable's value or nullptr if the index is not in range.
         */
        values::value const* get(size_t index) const;

        /**
         * Pushes the current match variables.
         */
        void push_matches();

        /**
         * Pops the current match variables.
         */
        void pop_matches();

     private:
        std::string _name;
        std::string _display_name;
        scope* _parent;
        std::unordered_map<std::string, std::tuple<values::value, size_t>> _variables;
        std::deque<std::vector<values::value>> _matches;
    };

    /**
     * Stream insertion operator for runtime scope.
     * @param os The output stream to write the runtime scope to.
     * @param s The runtime scope to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, scope const& s);

    /**
     * Represents a match variable scope.
     */
    struct match_variable_scope
    {
        /**
         * Constructs a match variable scope.
         * @param current The current scope.
         */
        explicit match_variable_scope(scope& current);

        /**
         * Destructs a match variable scope.
         */
        ~match_variable_scope();

     private:
        scope& _current;
    };

}}  // puppet::runtime
