/**
 * @file
 * Declares the binary operator context.
 */
#pragma once

#include "../../lexer/position.hpp"
#include "../values/value.hpp"
#include "../expression_evaluator.hpp"

namespace puppet { namespace runtime { namespace operators {

    /**
     * Represents context for invoking a binary operator.
     */
    struct binary_context
    {
        /**
         * Constructs a binary operator context.
         * @param evaluator The expression evaluator.
         * @param left The left operand to the operator.
         * @param left_position The position of the left operand in the input.
         * @param right The right operand to the operator.
         * @param right_position The position of the right operand in the input.
         */
        binary_context(expression_evaluator& evaluator, values::value& left, lexer::position const& left_position, values::value& right, lexer::position const& right_position);

        /**
         * Gets the expression evaluator.
         * @return Returns the expression evaluator.
         */
        expression_evaluator& evaluator();

        /**
         * Gets the left operand.
         * @return Returns the left operand.
         */
        values::value& left();

        /**
         * Gets the left operand.
         * @return Returns the left operand.
         */
        values::value const& left() const;

        /**
         * Gets the position of the left operand.
         * @return Returns the position of the left operand.
         */
        lexer::position const& left_position() const;

        /**
         * Gets the right operand.
         * @return Returns the right operand.
         */
        values::value& right();

        /**
         * Gets the right operand.
         * @return Returns the right operand.
         */
        values::value const& right() const;

        /**
         * Gets the position of the right operand.
         * @return Returns the position of the right operand.
         */
        lexer::position const& right_position() const;

    private:
        expression_evaluator& _evaluator;
        values::value& _left;
        lexer::position const& _left_position;
        values::value& _right;
        lexer::position const& _right_position;
    };

}}}  // puppet::runtime::operators
