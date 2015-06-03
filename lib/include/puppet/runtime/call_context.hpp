/**
 * @file
 * Declares the runtime call context.
 */
#pragma once

#include "values/value.hpp"
#include "executor.hpp"
#include "expression_evaluator.hpp"

namespace puppet { namespace runtime {

    /**
     * Represents runtime context about a function call.
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
            lexer::position const& position,
            boost::optional<std::vector<ast::expression>> const& arguments,
            boost::optional<ast::lambda> const& lambda,
            values::value* first_value = nullptr,
            ast::primary_expression const* first_expression = nullptr,
            lexer::position const* first_position = nullptr);

        /**
         * Gets the expression evaluator.
         * @return Returns the expression evaluator.
         */
        expression_evaluator& evaluator();

        /**
         * Gets the name of the function being called.
         * @return Returns the name of the function being called.
         */
        std::string const& name() const;

        /**
         * Gets the position of the function call itself.
         * @return Returns the position of the function call itself.
         */
        lexer::position const& position() const;

        /**
         * Gets the position of an argument to the function.
         * @param index The index of the argument.
         * @return Returns the position of the argument.
         */
        lexer::position const& position(size_t index) const;

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
         * Gets the executor for the lambda.
         * @return Returns the executor for the lambda.
         */
        runtime::executor const& lambda() const;

        /**
         * Determines if a lambda was passed to the function.
         * @return Returns true if a lambda was passed or false if one was not.
         */
        bool lambda_given() const;

    private:
        expression_evaluator& _evaluator;
        std::string const& _name;
        lexer::position const& _position;
        values::array _arguments;
        std::vector<lexer::position> _positions;
        runtime::executor _executor;
        bool _lambda_given;
    };

}}  // puppet::runtime
