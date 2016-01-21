/**
 * @file
 * Declares the function and operator call dispatcher.
 */
#pragma once

#include "functions/descriptor.hpp"
#include "operators/binary/descriptor.hpp"
#include "operators/unary/descriptor.hpp"
#include <unordered_map>

namespace puppet { namespace compiler { namespace evaluation {

    /**
     * Represents the function and operator call dispatcher.
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
         * Adds the built-in Puppet functions and operators to the dispatcher.
         */
        void add_builtins();

        /**
         * Adds a function to the dispatcher.
         * @param descriptor The descriptor of the function to add.
         */
        void add(functions::descriptor descriptor);

        /**
         * Adds a binary operator to the dispatcher.
         * @param descriptor The descriptor of the binary operator to add.
         */
        void add(operators::binary::descriptor descriptor);

        /**
         * Adds a unary operator to the dispatcher.
         * @param descriptor The descriptor of the unary operator to add.
         */
        void add(operators::unary::descriptor descriptor);

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
         * Finds a binary operator descriptor given the binary operator.
         * @param oper The binary operator to find.
         * @return Returns the binary operator descriptor or nullptr if not found.
         */
        operators::binary::descriptor* find(ast::binary_operator oper);

        /**
         * Finds a binary operator descriptor given the binary operator.
         * @param oper The binary operator to find.
         * @return Returns the binary operator descriptor or nullptr if not found.
         */
        operators::binary::descriptor const* find(ast::binary_operator oper) const;

        /**
         * Finds a unary operator descriptor given the unary operator.
         * @param oper The unary operator to find.
         * @return Returns the unary operator descriptor or nullptr if not found.
         */
        operators::unary::descriptor* find(ast::unary_operator oper);

        /**
         * Finds a unary operator descriptor given the unary operator.
         * @param oper The unary operator to find.
         * @return Returns the unary operator descriptor or nullptr if not found.
         */
        operators::unary::descriptor const* find(ast::unary_operator oper) const;

        /**
         * Dispatches a function call.
         * @param context The function call context to dispatch.
         * @return Returns the value returned by the called function.
         */
        runtime::values::value dispatch(functions::call_context& context) const;

        /**
         * Dispatches a binary operator call.
         * @param context The binary operator call context to dispatch.
         * @return Returns the value returned by the called operator.
         */
        runtime::values::value dispatch(operators::binary::call_context& context) const;

        /**
         * Dispatches a unary operator call.
         * @param context The unary operator call context to dispatch.
         * @return Returns the value returned by the called operator.
         */
        runtime::values::value dispatch(operators::unary::call_context& context) const;

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
        std::vector<operators::binary::descriptor> _binary_operators;
        std::vector<operators::unary::descriptor> _unary_operators;
    };

}}}  // namespace puppet::compiler::evaluation
