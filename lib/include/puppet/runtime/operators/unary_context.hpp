/**
 * @file
 * Declares the unary operator context.
 */
#pragma once

#include "../../lexer/position.hpp"
#include "../values/value.hpp"
#include "../expression_evaluator.hpp"

namespace puppet { namespace runtime { namespace operators {

    /**
     * Represents context for invoking a unary operator.
     */
    struct unary_context
    {
        /**
         * Constructs a unary operator context.
         * @param evaluator The expression evaluator.
         * @param operand The operand for the unary operator.
         * @param position The position of the operand.
         */
        unary_context(expression_evaluator& evaluator, values::value& operand, lexer::position const& position);

        /**
         * Gets the expression evaluator.
         * @return Returns the expression evaluator.
         */
        expression_evaluator& evaluator();

        /**
         * Gets the operand.
         * @return Returns the operand.
         */
        values::value& operand();

        /**
         * Gets the operand.
         * @return Returns the operand.
         */
        values::value const& operand() const;

        /**
         * Gets the position of the operand.
         * @return Returns the position of the operand.
         */
        lexer::position const& position() const;

    private:
        expression_evaluator& _evaluator;
        values::value& _operand;
        lexer::position const& _position;
    };

}}}  // puppet::runtime::operators
