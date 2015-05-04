/**
 * @file
 * Declares the unary operator context.
 */
#pragma once

#include "../../lexer/token_position.hpp"
#include "../values/value.hpp"
#include "../context.hpp"

namespace puppet { namespace runtime { namespace operators {

    /**
     * Represents context for invoking a unary operator.
     */
    struct unary_context
    {
        /**
         * Constructs a unary operator context.
         * @param evaluation_context The current evaluation context.
         * @param operand The operand for the unary operator.
         * @param position The position of the operand.
         */
        unary_context(context& evaluation_context, values::value& operand, lexer::token_position const& position);

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
        lexer::token_position const& position() const;

    private:
        values::value& _operand;
        lexer::token_position const& _position;
        context& _evaluation_context;
    };

}}}  // puppet::runtime::operators
