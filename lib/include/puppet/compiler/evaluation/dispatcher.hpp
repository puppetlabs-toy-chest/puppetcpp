/**
 * @file
 * Declares the function call dispatcher.
 */
#pragma once

#include "functions/descriptor.hpp"
#include <unordered_map>

namespace puppet { namespace compiler { namespace evaluation {

    /**
     * Represents the function call dispatcher.
     */
    struct dispatcher
    {
        /**
         * The function type for dispatch fallback.
         */
        using fallback_type = std::function<boost::optional<runtime::values::value>(functions::call_context& context)>;

        /**
         * Default constructor for dispatcher.
         */
        dispatcher() = default;

        /**
         * Default move constructor for dispatcher.
         */
        dispatcher(dispatcher&&) = default;

        /**
         * Default move assignment operator for dispatcher.
         * @return Returns this dispatcher.
         */
        dispatcher& operator=(dispatcher&&) = default;

        /**
         * Adds the built-in Puppet functions to the dispatcher.
         */
        void add_builtin_functions();

        /**
         * Adds a function to the dispatcher.
         * @param descriptor The descriptor of the function to add.
         */
        void add(functions::descriptor descriptor);

        /**
         * Finds a function by name.
         * @param name The function to find.
         * @return Returns the function descriptor if found or nullptr if not found.
         */
        functions::descriptor* find(std::string const& name);

        /**
         * Finds a function by name.
         * @param name The function to find.
         * @return Returns the function descriptor if found or nullptr if not found.
         */
        functions::descriptor const* find(std::string const& name) const;

        /**
         * Dispatches a function call.
         * @param context The function call context to dispatch.
         * @return Returns the value returned by the called function.
         */
        runtime::values::value dispatch(functions::call_context& context) const;

        /**
         * Set the fallback callback to use.
         * @param fallback The fallback callback to use.
         */
        void fallback(fallback_type fallback);

     private:
        dispatcher(dispatcher&) = delete;
        dispatcher& operator=(dispatcher&) = delete;

        fallback_type _fallback;
        std::unordered_map<std::string, functions::descriptor> _functions;
    };

}}}  // namespace puppet::compiler::evaluation
