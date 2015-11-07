/**
 * @file
 * Declares the call evaluator.
 */
#pragma once

#include "../ast/ast.hpp"
#include "../resource.hpp"
#include "scope.hpp"

namespace puppet { namespace compiler { namespace evaluation {

    /**
     * Forward declaration of context.
     */
    struct context;

    /**
     * Exception for arguments passed by index.
     */
    struct argument_exception : std::runtime_error
    {
        /**
         * Constructs a new argument exception.
         * @param message The exception message.
         * @param index The index of the argument.
         */
        argument_exception(std::string const& message, size_t index);

        /**
         * Gets the index of the argument that caused the exception.
         * @return Returns the index of the argument that caused the exception.
         */
        size_t index() const;

     private:
        size_t _index;
    };

    /**
     * Represents the call evaluator.
     * Responsible for evaluating function, defined type, class, and lambda calls.
     */
    struct call_evaluator
    {

        /**
         * Constructs a call evaluator.
         * @param context The current evaluation context.
         * @param parameters The parameters of the object being called.
         * @param body The body to evaluate.
         */
        call_evaluator(evaluation::context& context, std::vector<ast::parameter> const& parameters, std::vector<ast::expression> const& body);

        /**
         * Evaluates the call.
         * @param scope The scope to evaluate the call in.
         * @return Returns the result of evaluating the call.
         */
        runtime::values::value evaluate(std::shared_ptr<scope> const& scope = nullptr) const;

        /**
         * Evaluates the call.
         * @param arguments The arguments passed to the call.
         * @param scope The scope to evaluate the call in.
         * @return Returns the result of evaluating the call.
         */
        runtime::values::value evaluate(runtime::values::array& arguments, std::shared_ptr<scope> const& scope = nullptr) const;

        /**
         * Evaluates the call.
         * @param resource The resource with attributes that become arguments to the function.
         * @param scope The scope to evaluate the call in.
         * @return Returns the result of evaluating the call.
         */
        runtime::values::value evaluate(compiler::resource& resource, std::shared_ptr<scope> const& scope = nullptr) const;

     private:
        void validate_parameter_type(ast::parameter const& parameter, runtime::values::value const& value, std::function<void(std::string)> const& error) const;
        runtime::values::value evaluate_body() const;

        evaluation::context& _context;
        std::vector<ast::parameter> const& _parameters;
        std::vector<ast::expression> const& _body;
    };

}}}  // puppet::compiler::evaluation
