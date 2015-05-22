/**
 * @file
 * Declares the function dispatcher.
 */
#pragma once

#include "call_context.hpp"
#include "expression_evaluator.hpp"
#include <functional>

namespace puppet { namespace runtime {

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
        dispatcher(std::string const& name, lexer::position const& position);

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
            lexer::position const* first_position = nullptr) const;

        /**
         * Gets the name of the function being dispatched.
         * @return Returns the name of the function being dispatched.
         */
        std::string const& name() const;

        /**
         * Gets the position of the function call itself.
         * @return Returns the position of the function call itself.
         */
        lexer::position const& position() const;

     private:
        std::string const& _name;
        lexer::position const& _position;
        function_type const* _function;
    };

}}  // puppet::runtime
