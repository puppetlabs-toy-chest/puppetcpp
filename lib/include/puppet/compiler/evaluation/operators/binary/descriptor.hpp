/**
 * @file
 * Declares the binary operator descriptor.
 */
#pragma once

#include "../../../ast/ast.hpp"
#include "../../../../runtime/values/value.hpp"
#include <string>
#include <vector>
#include <functional>

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    // Forward declaration of binary operator call context.
    struct call_context;

    /**
     * Responsible for describing a Puppet binary operator.
     */
    struct descriptor
    {
        /**
         * The callback type to call when the operator call is dispatched.
         */
        using callback_type = std::function<runtime::values::value(call_context&)>;

        /**
         * Constructs a binary operator descriptor.
         * @param oper The binary operator represented by this descriptor.
         */
        explicit descriptor(ast::binary_operator oper);

        /**
         * Gets the operator represented by this descriptor.
         * @return Returns the operator represented by this descriptor.
         */
        ast::binary_operator oper() const;

        /**
         * Determines if the operator has dispatch descriptors.
         * @return Returns true if the operator has dispatch descriptors or false if not.
         */
        bool dispatchable() const;

        /**
         * Adds a dispatch descriptor for the operator.
         * @param left_type The type of the left-hand operand to the binary operator
         * @param right_type The type of the right-hand operand to the binary operator
         * @param callback The callback to invoke when the function call is dispatched.
         */
        void add(std::string const& left_type, std::string const& right_type, callback_type callback);

        /**
         * Dispatches an operator call to the matching dispatch descriptor.
         * @param context The binary operator context to dispatch.
         * @return Returns the result of the operator call.
         */
        runtime::values::value dispatch(binary::call_context& context) const;

     private:
        struct dispatch_descriptor
        {
            runtime::values::type left_type;
            runtime::values::type right_type;
            callback_type callback;
        };

        ast::binary_operator _operator;
        std::vector<dispatch_descriptor> _dispatch_descriptors;
    };

}}}}}  // puppet::compiler::evaluation::operators::binary
