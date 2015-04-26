/**
 * @file
 * Declares the Puppet language operators.
 */
#pragma once

#include "values/value.hpp"
#include "context.hpp"
#include "../lexer/token_position.hpp"

namespace puppet { namespace runtime {

    /**
     * Implements the "in" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param ctx The current evaluation context.
     * @return Return true if the left value is "in" the right value, depending on the actual types of the values.
     */
    bool in(values::value const& left, values::value const& right, context& ctx);

    /**
     * Implements the "assignment" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param ctx The current evaluation context.
     * @param position The position of the assignment expression.
     */
    void assign(values::value& left, values::value& right, context& ctx, lexer::token_position const& position);

    /**
     * Implements the "plus" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @return Returns the added value.
     */
    values::value plus(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position);

    /**
     * Implements the "minus" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @return Returns the subtracted value.
     */
    values::value minus(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position);

    /**
     * Implements the "multiply" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @return Returns the multiplied value.
     */
    values::value multiply(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position);

    /**
     * Implements the "division" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @return Returns the divided value.
     */
    values::value divide(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position);

    /**
     * Implements the "modulo" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @return Returns the modulo value.
     */
    values::value modulo(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position);

    /**
     * Implements the unary arithmetic negation operator.
     * @param operand The operand being negated.
     * @param position The position of the unary expression.
     * @return Returns the negated value.
     */
    values::value negate(values::value const& operand, lexer::token_position const& position);

    /**
     * Implements the "bitwise left shift" (or append for arrays) operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @return Returns the shifted (or appended) value.
     */
    values::value left_shift(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position);

    /**
     * Implements the "bitwise right shift" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @return Returns the shifted value.
     */
    values::value right_shift(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position);

    /**
     * Implements the "logical and" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true if both values are "truthy" or false if one or both are not.
     */
    values::value logical_and(values::value const& left, values::value const& right);

    /**
     * Implements the "logical or" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true if one or both values are "truthy" or false if neither is truthy.
     */
    values::value logical_or(values::value const& left, values::value const& right);

    /**
     * Implements the unary "logical not" operator.
     * @param operand The operand.
     * @return Returns the logical not of the value.
     */
    values::value logical_not(values::value const& operand);

    /**
     * Implements the "less than" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @return Returns true if left is less than right or false if not.
     */
    values::value less(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position);

    /**
     * Implements the "less than or equal to" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @return Returns true if left is less than or equal to right or false if not.
     */
    values::value less_equal(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position);

    /**
     * Implements the "greater than" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @return Returns true if left is greater than right or false if not.
     */
    values::value greater(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position);

    /**
     * Implements the "greater than or equal to" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @return Returns true if left is greater than or equal to right or false if not.
     */
    values::value greater_equal(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position);

    /**
     * Implements the "match" operator.
     * @param left The left operand.
     * @param right The right operand.
     * @param left_position The position of the left operand.
     * @param right_position The position of the right operand.
     * @param ctx The current evaluation context.
     * @return Returns true if the left matches the right or false if not.
     */
    values::value match(values::value const& left, values::value const& right, lexer::token_position const& left_position, lexer::token_position const& right_position, context& ctx);

    /**
     * Implements the "splat" operator.
     * Note: the splat operator performs the "to array" behavior; unfolding is implemented in the expression evaluator.
     * @param operand The operand to splat.
     * @return Returns the splatted value.
     */
    values::value splat(values::value operand);

}}  // puppet::runtime
