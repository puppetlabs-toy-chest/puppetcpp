/**
 * @file
 * Declares the lambda yielder.
 */
#pragma once

#include "../ast/lambda.hpp"
#include "value.hpp"
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
         * Determines if the lambda was given to the function.
         * @return Returns true if the lambda was given or false if not.
         */
        bool lambda_given() const;

        /**
         * Yields to the lambda without passing any arguments.
         * @return Returns the value from the lambda's block.
         */
        value yield();

        /**
         * Yields to the lambda with the given arguments.
         * Note: elements of the array may be mutated.
         * @param arguments The arguments to pass to the lambda.
         * @return Returns the value from the lambda's block.
         */
        value yield(array& arguments);

     private:
        expression_evaluator& _evaluator;
        lexer::token_position const& _position;
        boost::optional<ast::lambda> const& _lambda;
        std::unordered_map<std::string, value> _default_cache;
    };

}}  // puppet::runtime
