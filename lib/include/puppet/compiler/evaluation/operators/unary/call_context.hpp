/**
 * @file
 * Declares the unary operator call context.
 */
#pragma once

#include "../../context.hpp"
#include "../../../ast/ast.hpp"
#include "../../../../runtime/values/value.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace unary {

    /**
     * Represents call context for calling a unary operator.
     */
    struct call_context
    {
        /**
         * Constructs a unary operator call context.
         * @param context The evaluation context.
         * @param oper The unary operator being called.
         * @param operator_context The AST context of the operator.
         * @param operand The operand to the operator.
         * @param operand_context The AST context of the operand.
         */
        call_context(
            evaluation::context& context,
            ast::unary_operator oper,
            ast::context const& operator_context,
            runtime::values::value& operand,
            ast::context const& operand_context);

        /**
         * Gets the evaluation context.
         * @return Returns the evaluation context.
         */
        evaluation::context& context() const;

        /**
         * Gets the unary operator being called.
         * @return Returns the unary operator being called.
         */
        ast::unary_operator oper() const;

        /**
         * Gets the AST context of the unary operator.
         * @return Returns the AST context of the unary operator.
         */
        ast::context const& operator_context() const;

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
        ast::unary_operator _operator;
        ast::context const& _operator_context;
        runtime::values::value& _operand;
        ast::context const& _operand_context;
    };

}}}}}  // namespace puppet::compiler::evaluation::operators::unary
