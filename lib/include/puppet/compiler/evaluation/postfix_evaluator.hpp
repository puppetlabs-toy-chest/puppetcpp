/**
 * @file
 * Declares the Puppet language postfix expression evaluator.
 */
#pragma once

#include "context.hpp"
#include "../ast/ast.hpp"
#include "../../runtime/values/value.hpp"

namespace puppet { namespace compiler { namespace evaluation {

    /**
     * Represents the Puppet language postfix expression evaluator.
     */
    struct postfix_evaluator
    {
        /**
         * Constructs a postfix expression evaluator.
         * @param context The current evaluation context.
         */
        explicit postfix_evaluator(evaluation::context& context);

        /**
         * Evaluates a postfix expression.
         * @param expression The expression to evaluate.
         * @return Returns the value resulting in evaluating the postfix expression.
         */
        runtime::values::value evaluate(ast::postfix_expression const& expression);

     private:
        context& _context;
    };

}}}  // namespace puppet::compiler::evaluation
