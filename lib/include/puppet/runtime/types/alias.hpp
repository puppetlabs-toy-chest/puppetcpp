/**
 * @file
 * Declares the type alias.
 */
#pragma once

#include "../values/forward.hpp"
#include <ostream>
#include <unordered_map>

namespace puppet { namespace runtime { namespace types {

    // Forward declaration of recursion_guard
    struct recursion_guard;

    /**
     * Represents a Puppet type alias.
     */
    struct alias
    {
        /**
         * Constructs a Puppet type alias.
         * @param name The type alias name.
         * @param resolved_type The type the alias resolved to.
         */
        alias(std::string name, std::shared_ptr<values::type> resolved_type);

        /**
         * Gets the name of the type alias.
         * @return Returns the name of the type alias.
         */
        std::string const& name() const;

        /**
         * Gets the resolved type of the alias.
         * @return Returns the resolved type of the alias.
         */
        values::type const& resolved_type() const;

        /**
         * Creates a generalized version of the type.
         * @return Returns the generalized type.
         */
        values::type generalize() const;

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

     private:
        std::string _name;
        std::shared_ptr<values::type> _resolved_type;
    };

    /**
     * Stream insertion operator for alias type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, alias const& type);

    /**
     * Equality operator for alias.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Always returns true (Any type is always equal to Any).
     */
    bool operator==(alias const& left, alias const& right);

    /**
     * Inequality operator for alias.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Always returns false (Any type is always equal to Any).
     */
    bool operator!=(alias const& left, alias const& right);

    /**
     * Hashes the alias type.
     * @param type The alias type to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(alias const& type);

    /**
     * Responsible for guarding against type alias recursion.
     */
    struct recursion_guard
    {
        /**
         * Represents the recursion guard map key.
          */
        struct key
        {
            /**
             * Constructs a recursion guard map key.
             * @param alias The type alias being guarded.
             * @param other The pointer to the other thing being compared against; nullptr if the alias is not being compared to something else.
             */
            explicit key(types::alias const& alias, void const* other = nullptr);

            /**
             * Gets the alias' resolved type.
             * @return Returns the alias' resolved type.
             */
            values::type const& resolved() const;

            /**
             *  Gets the other thing the alias is being compared to.
             *  @return Returns the other thing the alias is being compared to or nullptr if not being compared.
             */
            void const* other() const;

         private:
            values::type const& _resolved;
            void const* _other;
        };

        /**
         * The map type for the recursion guard.
         */
        using map_type = std::unordered_map<key, bool, boost::hash<key>>;

        /**
         * Represents the result when adding an alias to the guard.
         */
        struct result
        {
            /**
             * Gets whether or not the type alias was recursed.
             * @return Returns true if the type alias was recursed or false if not.
             */
            bool recursed() const;

            /**
             * Gets the current result value.
             * @return Returns the current result value.
             */
            bool value() const;

            /**
             * Sets the current result value for the alias.
             * @param val The new result value for the alias.
             */
            void value(bool val);

         private:
            friend struct recursion_guard;
            result(map_type::iterator iterator, bool recursed);

            map_type::iterator _iterator;
            bool _recursed;
        };

        /**
         * Adds an alias to the guard.
         * @param alias The alias to add.
         * @param other The other thing being compared against; nullptr to indicate the type alias is not being compared against.
         * @return Returns a result object.
         */
        result add(types::alias const& alias, void const* other = nullptr);

     private:
        map_type _map;
    };

    /**
     * Compares two recursion guard keys.
     * @param left The left recursion guard key to compare.
     * @param right The right recursion guard key to compare.
     * @return Returns true if the two keys are equal or false if they are not equal.
     */
    bool operator==(recursion_guard::key const& left, recursion_guard::key const& right);

    /**
     * Hashes a recursion guard key.
     * @param key The recursion guard key to hash.
     * @return Returns the hash value.
     */
    size_t hash_value(recursion_guard::key const& key);

}}}  // namespace puppet::runtime::types
