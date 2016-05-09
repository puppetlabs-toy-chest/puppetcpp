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
    struct call_context
    {
        /**
         * Constructs a function call context from a function call expression.
         * @param context The current evaluation context.
         * @param expression The function call expression.
         */
        call_context(evaluation::context& context, ast::function_call_expression const& expression);

        /**
         * Constructs a function call context from a method call expression.
         * @param context The current evaluation context.
         * @param expression The method call expression.
         * @param instance The value the method is being called on.
         * @param instance_context The AST context for the instance.
         * @param splat True if splatting of the instance value is supported or false if not.
         */
        call_context(evaluation::context& context, ast::method_call_expression const& expression, runtime::values::value& instance, ast::context const& instance_context, bool splat);

        /**
         * Constructs a function call context from a new expression.
         * @param context The current evaluation context.
         * @param expression The new expression.
         * @param name The AST function name to use for the call.
         */
        call_context(evaluation::context& context, ast::new_expression const& expression, ast::name const& name);

        /**
         * Gets the current evaluation context.
         * @return Returns the current evaluation context.
         */
        evaluation::context& context() const;

        /**
         * Gets the AST name of the function being called.
         * @return Returns the AST name of the function being called.
         */
        ast::name const& name() const;

        /**
         * Gets the arguments to the function.
         * @return Returns the arguments to the function.
         */
        runtime::values::array& arguments();

        /**
         * Gets the arguments to the function.
         * @return Returns the arguments to the function.
         */
        runtime::values::array const& arguments() const;

        /**
         * Gets an argument to the function.
         * @param index The argument index.
         * @return Returns the argument.
         */
        runtime::values::value& argument(size_t index);

        /**
         * Gets an argument to the function.
         * @param index The argument index.
         * @return Returns the argument.
         */
        runtime::values::value const& argument(size_t index) const;

        /**
         * Gets the AST context of an argument to the function.
         * @param index The argument index.
         * @return Returns the AST context of the argument.
         */
        ast::context const& argument_context(size_t index) const;

        /**
         * Gets the optional block passed to the function.
         * @return Returns the optional block passed to the function or nullptr if no block were passed.
         */
        boost::optional<ast::lambda_expression> const& block() const;

        /**
         * Yields to the block if one is present.
         * @param arguments The arguments to yield to the block.
         * @return Returns the value that was returned by the block.
         */
        runtime::values::value yield(runtime::values::array& arguments) const;

        /**
         * Yields to the block if one is present, without catching argument exceptions.
         * @param arguments The arguments to yield to the block.
         * @return Returns the value that was returned by the block.
         */
        runtime::values::value yield_without_catch(runtime::values::array& arguments) const;

    private:
        void evaluate_arguments(std::vector<ast::expression> const& arguments);

        evaluation::context& _context;
        ast::name const& _name;
        runtime::values::array _arguments;
        std::vector<ast::context> _argument_contexts;
        boost::optional<ast::lambda_expression> const& _block;
        std::shared_ptr<scope> _closure_scope;
    };

}}}}  // namespace puppet::compiler::evaluation::functions
