/**
 * @file
 * Declares the lambda yielder.
 */
#pragma once

#include "../ast/lambda.hpp"
#include "values/value.hpp"
#include "expression_evaluator.hpp"
#include <boost/optional.hpp>
#include <vector>
#include <functional>
#include <unordered_map>
#include <string>

namespace puppet { namespace runtime {

    /**
     * Represents the lambda yielder.
     */
    struct yielder
    {
        /**
         * Constructs a lambda yielder.
         * @param evaluator The current expression evaluator.
         * @param position The position of the function call that was passed the lambda.
         * @param lambda The AST lambda being yielded to.
         */
        yielder(expression_evaluator& evaluator, lexer::token_position const& position, boost::optional<ast::lambda> const& lambda);

        /**
         * Gets the position of the lambda itself.
         * @return Returns the position of the lambda itself.
         */
        lexer::token_position const& position() const;

        /**
         * Gets the position of an parameter to the lambda.
         * @param index The index of the parameter.
         * @return Returns the position of the parameter.
         */
        lexer::token_position const& position(size_t index) const;

        /**
         * Determines if the lambda was given to the function.
         * @return Returns true if the lambda was given or false if not.
         */
        bool lambda_given() const;

        /**
         * Gets the count of parameters to the lambda.
         * @return Returns the count of parameters to the lambda.
         */
        size_t parameter_count() const;

        /**
         * Yields to the lambda without passing any arguments.
         * @return Returns the value from the lambda's block.
         */
        values::value yield() const;

        /**
         * Yields to the lambda with the given arguments.
         * @param arguments The arguments to pass to the lambda.
         * @return Returns the value from the lambda's block.
         */
        values::value yield(values::array& arguments) const;

     private:
        expression_evaluator& _evaluator;
        lexer::token_position const& _position;
        boost::optional<ast::lambda> const& _lambda;
    };

}}  // puppet::runtime
