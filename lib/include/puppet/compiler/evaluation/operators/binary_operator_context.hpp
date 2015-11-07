/**
 * @file
 * Declares the binary operator context.
 */
#pragma once

#include "../context.hpp"
#include "../../ast/ast.hpp"
#include "../../../runtime/values/value.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    /**
     * Represents context for invoking a binary operator.
     */
    struct binary_operator_context
    {
        /**
         * Constructs a binary operator context.
         * @param context The evaluation context.
         * @param left The left operand to the operator.
         * @param left_context The AST context of the left operand.
         * @param right The right operand to the operator.
         * @param right_context The AST context of the right operand.
         */
        binary_operator_context(
            evaluation::context& context,
            runtime::values::value& left,
            ast::context const& left_context,
            runtime::values::value& right,
            ast::context const& right_context);

        /**
         * Gets the evaluation context.
         * @return Returns the evaluation context.
         */
        evaluation::context& context() const;

        /**
         * Gets the left operand.
         * @return Returns the left operand.
         */
        runtime::values::value& left() const;

        /**
         * Gets the AST context of the left operand.
         * @return the AST context of the left operand.
         */
        ast::context const& left_context() const;

        /**
         * Gets the right operand.
         * @return Returns the right operand.
         */
        runtime::values::value& right() const;

        /**
         * Gets the AST context of the right operand.
         * @return the AST context of the right operand.
         */
        ast::context const& right_context() const;

     private:
        evaluation::context& _context;
        runtime::values::value& _left;
        ast::context const& _left_context;
        runtime::values::value& _right;
        ast::context const& _right_context;
    };

}}}}  // namespace puppet::compiler::evaluation::operators