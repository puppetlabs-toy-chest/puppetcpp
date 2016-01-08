/**
 * @file
 * Declares the function descriptor.
 */
#pragma once

#include "../../../runtime/values/value.hpp"
#include <string>
#include <vector>
#include <functional>

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    // Forward declaration of function call_context.
    struct call_context;

    /**
     * Responsible for describing a Puppet function.
     */
    struct descriptor
    {
        /**
         * The callback type to call when the function call is dispatched.
         */
        using callback_type = std::function<runtime::values::value(functions::call_context&)>;

        /**
         * Constructs a function descriptor.
         * @param name The name of the function.
         */
        explicit descriptor(std::string name);

        /**
         * Gets the function's name.
         * @return Returns the function's name.
         */
        std::string const& name() const;

        /**
         * Determines if the function has dispatch descriptors.
         * @return Returns true if the function has dispatch descriptors or false if not.
         */
        bool dispatchable() const;

        /**
         * Adds a dispatch descriptor for the function.
         * @param signature The signature for function call dispatch.
         * @param callback The callback to invoke when the function call is dispatched.
         */
        void add(std::string const& signature, callback_type callback);

        /**
         * Dispatches a function call to the matching dispatch descriptor.
         * @param context The call context to dispatch.
         * @return Returns the result of the function call.
         */
        runtime::values::value dispatch(call_context& context) const;

     private:
        struct dispatch_descriptor
        {
            runtime::types::callable signature;
            callback_type callback;
        };

        std::vector<dispatch_descriptor const*> check_argument_count(call_context const& context) const;
        void check_block_parameters(call_context const& context, std::vector<dispatch_descriptor const*> const& invocable) const;
        void check_parameter_types(call_context const& context, std::vector<dispatch_descriptor const*> const& invocable) const;

        std::string _name;
        std::vector<dispatch_descriptor> _dispatch_descriptors;
    };

}}}}  // puppet::compiler::evaluation::functions
