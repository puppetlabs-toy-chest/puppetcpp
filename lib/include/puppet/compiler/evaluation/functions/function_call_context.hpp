/**
 * @file
 * Declares the function call context.
 */
#pragma once

#include "../context.hpp"
#include "../../ast/ast.hpp"
#include "../../../runtime/values/value.hpp"
#include <boost/optional.hpp>

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    /**
     * Represents context for a function call.
     */
    struct function_call_context
    {
        /**
         * Constructs a function call context from a function call expression.
         * @param context The current evaluation context.
         * @param expression The function call expression.
         */
        explicit function_call_context(evaluation::context& context, ast::function_call_expression const& expression);

        /**
         * Constructs a function call context from a method call expression.
         * @param context The current evaluation context.
         * @param expression The method call expression.
         * @param instance The value the method is being called on.
         * @param instance_context The AST context of the instance.
         * @param splat True if splatting of the instance value is supported or false if not.
         */
        explicit function_call_context(evaluation::context& context, ast::method_call_expression const& expression, runtime::values::value& instance, ast::context const& instance_context, bool splat);

        /**
         * Gets the current evaluation context.
         * @return Returns the current evaluation context.
         */
        evaluation::context& context() const;

        /**
         * Gets the name of the function being called.
         * @return Returns the name of the function being called.
         */
        std::string const& name() const;

        /**
         * Gets the AST context of the call site.
         * @return Returns the AST context of the call site.
         */
        ast::context const& call_site() const;

        /**
         * Gets the arguments to the function.
         * @return Returns the arguments to the function.
         */
        runtime::values::array& arguments();

        /**
         * Gets an argument to the function.
         * @param index The argument index.
         * @return Returns the argument.
         */
        runtime::values::value& argument(size_t index);

        /**
         * Gets the AST context of an argument to the function.
         * @param index The argument index.
         * @return Returns the AST context of the argument.
         */
        ast::context const& argument_context(size_t index) const;

        /**
         * Gets the optional lambda passed to the function.
         * @return Returns the optional lambda passed to the function.
         */
        boost::optional<ast::lambda_expression> const& lambda() const;

        /**
         * Yields to the lambda if one is present.
         * @param arguments The arguments to yield to the lambda.
         * @return Returns the value that was returned by the lambda.
         */
        runtime::values::value yield(runtime::values::array& arguments) const;

        /**
         * Yields to the lambda if one is present, without catching argument exceptions.
         * @param arguments The arguments to yield to the lambda.
         * @return Returns the value that was returned by the lambda.
         */
        runtime::values::value yield_without_catch(runtime::values::array& arguments) const;

    private:
        void evaluate_arguments(std::vector<ast::expression> const& arguments);

        evaluation::context& _context;
        std::string const& _name;
        ast::context const& _call_site;
        runtime::values::array _arguments;
        std::vector<ast::context const*> _argument_contexts;
        boost::optional<ast::lambda_expression> const& _lambda;
    };


}}}}  // namespace puppet::compiler::evaluation::functions
