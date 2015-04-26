/**
 * @file
 * Declares the Puppet language expression evaluator.
 */
#pragma once

#include "../ast/expression.hpp"
#include "context.hpp"
#include <cstdint>
#include <vector>
#include <exception>

namespace puppet { namespace runtime {

    /**
     * Exception for evaluation errors.
     */
    struct evaluation_exception : std::runtime_error
    {
        /**
         * Constructs an evaluator exception.
         * @param position The token position where evaluation failed.
         * @param message The exception message.
         */
        evaluation_exception(lexer::token_position position, std::string const& message);

        /**
         * Gets the token position where evaluation failed.
         * @return Returns the token position where evaluation failed.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
    };

    /**
     * Represents the Puppet language expression evaluator.
     */
    struct expression_evaluator
    {
        /**
         * Constructs the expression evaluator with the given evaluation context.
         * @param ctx The evaluation context to use.
         */
        explicit expression_evaluator(context& ctx);

        /**
         * Evaluates the given AST expression and returns the resulting runtime value.
         * @param expr The AST expression to evaluate.
         * @param productive True if the expression is required to be productive (i.e. has side effect) or false if not.
         * @return Returns the runtime value that is the result of evaluating the expression.
         */
        values::value evaluate(ast::expression const& expr, bool productive = false);

        /**
         * Evaluates the given AST primary expression and returns the resulting runtime value.
         * @param expr The AST primary expression to evaluate.
         * @return Returns the runtime value that is the result of evaluating the expression.
         */
        values::value evaluate(ast::primary_expression const& expr);

        /**
         * Gets the evaluation context.
         * @return Returns the evaluation context.
         */
        runtime::context const& context() const;

        /**
         * Gets the evaluation context.
         * @return Returns the evaluation context.
         */
        runtime::context& context();

        /**
         * Unfolds a splat expression.
         * @param expression The expression to check for splat unfolding.
         * @param evaluated The evaluated value of the given expression.
         * @return Returns the unfolded array if the expression is a splat or nullptr if not.
         */
        boost::optional<values::array> unfold(ast::expression const& expression, values::value& evaluated);

        /**
         * Unfolds a splated expression.
         * @param expression The expression to check for splat unfolding.
         * @param evaluated The evaluated value of the given expression.
         * @return Returns the unfolded array if the expression is a splat or nullptr if not.
         */
        boost::optional<values::array> unfold(ast::primary_expression const& expression, values::value& evaluated);

    private:
        static bool is_productive(ast::expression const& expr);

        void climb_expression(
            values::value& left,
            lexer::token_position& left_position,
            std::uint8_t min_precedence,
            std::vector<ast::binary_expression>::const_iterator& begin,
            std::vector<ast::binary_expression>::const_iterator const& end);

        void evaluate(
            values::value& left,
            lexer::token_position const& left_position,
            ast::binary_operator op,
            values::value& right,
            lexer::token_position& right_position);

        static uint8_t get_precedence(ast::binary_operator op);

        static bool is_right_associative(ast::binary_operator op);

        runtime::context& _context;
    };

}}  // namespace puppet::runtime