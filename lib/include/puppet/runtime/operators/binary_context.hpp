/**
 * @file
 * Declares the binary operator context.
 */
#pragma once

#include "../../lexer/token_position.hpp"
#include "../values/value.hpp"
#include "../context.hpp"

namespace puppet { namespace runtime { namespace operators {

    /**
     * Represents context for invoking a binary operator.
     */
    struct binary_context
    {
        /**
         * Constructs a binary operator context.
         * @param evaluation_context The current evaluation context.
         * @param left The left operand to the operator.
         * @param left_position The position of the left operand in the input.
         * @param right The right operand to the operator.
         * @param right_position The position of the right operand in the input.
         */
        binary_context(context& evaluation_context, values::value& left, lexer::token_position const& left_position, values::value& right, lexer::token_position const& right_position);

        /**
         * Gets the current evaluation context.
         * @return Returns the current evaluation context.
         */
        context& evaluation_context();

        /**
         * Gets the current evaluation context.
         * @return Returns the current evaluation context.
         */
        context const& evaluation_context() const;

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
        lexer::token_position const& left_position() const;

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
        lexer::token_position const& right_position() const;

    private:
        values::value& _left;
        lexer::token_position const& _left_position;
        values::value& _right;
        lexer::token_position const& _right_position;
        context& _evaluation_context;
    };

}}}  // puppet::runtime::operators
