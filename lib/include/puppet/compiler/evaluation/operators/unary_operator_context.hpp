/**
 * @file
 * Declares the unary operator context.
 */
#pragma once

#include "../context.hpp"
#include "../../ast/ast.hpp"
#include "../../../runtime/values/value.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    /**
     * Represents context for invoking a unary operator.
     */
    struct unary_operator_context
    {
        /**
         * Constructs a unary operator context.
         * @param context The evaluation context.
         * @param operand The operand to the operator.
         * @param operand_context The AST context of the operand.
         */
        unary_operator_context(
            evaluation::context& context,
            runtime::values::value& operand,
            ast::context const& operand_context);

        /**
         * Gets the evaluation context.
         * @return Returns the evaluation context.
         */
        evaluation::context& context() const;

        /**
         * Gets the operand.
         * @return Returns the operand.
         */
        runtime::values::value& operand() const;

        /**
         * Gets the AST context of the operand.
         * @return the AST context of the operand.
         */
        ast::context const& operand_context() const;

     private:
        evaluation::context& _context;
        runtime::values::value& _operand;
        ast::context const& _operand_context;
    };

}}}}  // namespace puppet::compiler::evaluation::operators