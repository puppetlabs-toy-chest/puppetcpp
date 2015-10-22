/**
 * @file
 * Declares the function call dispatcher.
 */
#pragma once

#include "../../runtime/values/value.hpp"
#include <functional>
#include <unordered_map>

namespace puppet { namespace compiler { namespace evaluation {

    namespace functions {

        // Forward declaration of function_call_context.
        struct function_call_context;

    }  // namespace puppet::compiler::evaluation::functions

    /**
     * Represents the function call dispatcher.
     */
    struct dispatcher
    {
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
         * Dispatches a function call.
         * @param context The function call context to dispatch.
         * @return Returns the value returned by the called function.
         */
        runtime::values::value dispatch(functions::function_call_context& context) const;

     private:
        dispatcher(dispatcher&) = delete;
        dispatcher& operator=(dispatcher&) = delete;
    };

}}}  // namespace puppet::compiler::evaluation
