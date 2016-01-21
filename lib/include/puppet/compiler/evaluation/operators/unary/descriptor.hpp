/**
 * @file
 * Declares the unary operator descriptor.
 */
#pragma once

#include "../../../ast/ast.hpp"
#include "../../../../runtime/values/value.hpp"
#include <string>
#include <vector>
#include <functional>

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace unary {

    // Forward declaration of unary operator call context.
    struct call_context;

    /**
     * Responsible for describing a Puppet unary operator.
     */
    struct descriptor
    {
        /**
         * The callback type to call when the operator call is dispatched.
         */
        using callback_type = std::function<runtime::values::value(call_context&)>;

        /**
         * Constructs a unary operator descriptor.
         * @param oper The unary operator represented by this descriptor.
         */
        explicit descriptor(ast::unary_operator oper);

        /**
         * Gets the operator represented by this descriptor.
         * @return Returns the operator represented by this descriptor.
         */
        ast::unary_operator oper() const;

        /**
         * Determines if the operator has dispatch descriptors.
         * @return Returns true if the operator has dispatch descriptors or false if not.
         */
        bool dispatchable() const;

        /**
         * Adds a dispatch descriptor for the operator.
         * @param type The type of the operand to the unary operator.
         * @param callback The callback to invoke when the function call is dispatched.
         */
        void add(std::string const& type, callback_type callback);

        /**
         * Dispatches an operator call to the matching dispatch descriptor.
         * @param context The unary operator context to dispatch.
         * @return Returns the result of the operator call.
         */
        runtime::values::value dispatch(unary::call_context& context) const;

     private:
        struct dispatch_descriptor
        {
            runtime::values::type type;
            callback_type callback;
        };

        ast::unary_operator _operator;
        std::vector<dispatch_descriptor> _dispatch_descriptors;
    };

}}}}}  // puppet::compiler::evaluation::operators::unary
