/**
 * @file
 * Declares the runtime evaluation context.
 */
#pragma once

#include "value.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <tuple>
#include <cstdint>
#include <regex>
#include <boost/optional.hpp>

namespace puppet { namespace runtime {

    /**
     * Represents a runtime scope.
     */
    struct scope
    {
        /**
         * Constructs a scope.
         * @param name The name of the scope.
         * @param parent The parent scope.
         */
        scope(std::string name, std::shared_ptr<scope> parent = nullptr);

        /**
         * Gets the name of the scope.
         * @return Returns the name of the scope.
         */
        std::string const& name() const;

        /**
         * Sets a variable in the scope.
         * @param name The name of the variable.
         * @param val The value of the variable.
         * @param line The line where the variable was assigned.
         * @return Returns a pointer to the value that was set or nullptr if the value already exists in this scope.
         */
        value const* set(std::string name, value val, std::uint32_t line);

        /**
         * Gets a variable in the scope.
         * @param name The name of the variable to get.
         * @return Returns the value of the variable or nullptr if the value does not exist.
         */
        value const* get(std::string const& name) const;

        /**
         * Determines the line where a variable was set.
         * @param name The name of the variable.
         * @return Returns the line where the variable was set (0 if from facts) or nullptr if no such variable.
         */
        boost::optional<uint32_t> where(std::string const& name) const;

        /**
         * Gets the parent scope.
         * @return Returns the parent scope.
         */
        scope const* parent() const;

        /**
         * Sets the given matches into the scope.
         * This will set the $0 - $n variables.
         * @param matches The matches to set.
         */
        void set(std::smatch const& matches);

     private:
        std::string _name;
        std::shared_ptr<scope> _parent;
        std::unordered_map<std::string, std::tuple<value, std::uint32_t>> _variables;
        std::unordered_map<std::string, value> _matches;
    };

    /**
     * Stream insertion operator for runtime scope.
     * @param os The output stream to write the runtime scope to.
     * @param s The runtime scope to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, scope const& s);

}}  // puppet::runtime