/**
 * @file
 * Declares the function dispatcher.
 */
#pragma once

#include "yielder.hpp"
#include "expression_evaluator.hpp"
#include <functional>

namespace puppet { namespace runtime {

    /**
     * Represents context about a function call.
     */
    struct call_context
    {
        /**
         * Constructs a call context.
         * @param evaluator The expression evaluator to use.
         * @param name The name of the function being called.
         * @param position The position of the function call.
         * @param arguments The arguments to the function.
         * @param lambda The lambda to the function.
         * @param first_value The first evaluated value (for method calls; nullptr otherwise).
         * @param first_expression The first expression (for method calls; nullptr otherwise).
         * @param first_position The first argument position (for method calls; nullptr otherwise).
         */
        call_context(
            expression_evaluator& evaluator,
            std::string const& name,
            lexer::token_position const& position,
            boost::optional<std::vector<ast::expression>> const& arguments,
            boost::optional<ast::lambda> const& lambda,
            values::value* first_value = nullptr,
            ast::primary_expression const* first_expression = nullptr,
            lexer::token_position const* first_position = nullptr);

        /**
         * Gets the name of the function being called.
         * @return Returns the name of the function being called.
         */
        std::string const& name() const;

        /**
         * Gets the position of the function call itself.
         * @return Returns the position of the function call itself.
         */
        lexer::token_position const& position() const;

        /**
         * Gets the position of an argument to the function.
         * @param index The index of the argument.
         * @return Returns the position of the argument.
         */
        lexer::token_position const& position(size_t index) const;

        /**
         * Gets the arguments.
         * @return Returns the arguments.
         */
        values::array const& arguments() const;

        /**
         * Gets the arguments.
         * @return Returns the arguments.
         */
        values::array& arguments();

        /**
         * Gets the yielder for a lambda.
         * @return Returns the yielder for a lambda.
         */
        runtime::yielder const& yielder() const;

        /**
         * Gets the yielder for a lambda.
         * @return Returns the yielder for a lambda.
         */
        runtime::yielder& yielder();

        /**
         * Gets the current evaluation context.
         * @return Returns the current evaluation context.
         */
        context const& evaluation_context() const;

        /**
         * Gets the current evaluation context.
         * @return Returns the current evaluation context.
         */
        context& evaluation_context();

      private:
        std::string const& _name;
        lexer::token_position const& _position;
        context& _context;
        values::array _arguments;
        std::vector<lexer::token_position> _positions;
        runtime::yielder _yielder;
    };

    /**
     * Represents the function dispatcher.
     */
    struct dispatcher
    {
        /**
         * Represents the runtime function type (signature of all functions).
         */
        typedef std::function<values::value(call_context&)> function_type;

        /**
         * Constructs a dispatcher for a function.
         * @param name The function name to dispatch.
         * @param position The position of the function call.
         */
        dispatcher(std::string const& name, lexer::token_position const& position);

        /**
         * Dispatches the function call.
         * @param evaluator The expression evaluator to use.
         * @param arguments The arguments to the function.
         * @param lambda The lambda to call for the function.
         * @param first_value The first evaluated value (for method calls; nullptr otherwise).
         * @param first_expression The first expression (for method calls; nullptr otherwise).
         * @param first_position The first argument position (for method calls; nullptr otherwise).
         * @return Returns the value returned from the function.
         */
        values::value dispatch(
            expression_evaluator& evaluator,
            boost::optional<std::vector<ast::expression>> const& arguments,
            boost::optional<ast::lambda> const& lambda,
            values::value* first_value = nullptr,
            ast::primary_expression const* first_expression = nullptr,
            lexer::token_position const* first_position = nullptr) const;

        /**
         * Gets the name of the function being dispatched.
         * @return Returns the name of the function being dispatched.
         */
        std::string const& name() const;

        /**
         * Gets the position of the function call itself.
         * @return Returns the position of the function call itself.
         */
        lexer::token_position const& position() const;

     private:
        std::string const& _name;
        lexer::token_position const& _position;
        function_type const* _function;
    };

}}  // puppet::runtime
