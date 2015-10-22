/**
 * @file
 * Declares the Puppet language access expression evaluator.
 */
#pragma once

#include "context.hpp"
#include "../ast/ast.hpp"
#include "../../runtime/values/value.hpp"

namespace puppet { namespace compiler { namespace evaluation {

    /**
     * Represents the Puppet language access expression evaluator.
     */
    struct access_evaluator
    {
        /**
         * Constructs a access expression evaluator.
         * @param context The current evaluation context.
         */
        explicit access_evaluator(evaluation::context& context);

        /**
         * Evaluates a access expression.
         * @param target The target value being accessed.
         * @param expression The expression to evaluate.
         * @return Returns the value resulting in evaluating the access expression.
         */
        runtime::values::value evaluate(runtime::values::value const& target, ast::access_expression const& expression);

    private:
        context& _context;
    };

}}}  // namespace puppet::compiler::evaluation
