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
         * @return Returns the runtime value that is the result of evaluating the expression.
         */
        value evaluate(ast::expression& expr);

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

     private:
        void climb_expression(
            value& left,
            lexer::token_position& left_position,
            std::uint8_t min_precedence,
            std::vector<ast::binary_expression>::iterator& begin,
            std::vector<ast::binary_expression>::iterator const& end
        );

        void evaluate(
            value& left,
            lexer::token_position const& left_position,
            ast::binary_operator op,
            value& right,
            lexer::token_position& right_position);

        static uint8_t get_precedence(ast::binary_operator op);
        static bool is_right_associative(ast::binary_operator op);

        runtime::context& _context;
    };

}}  // namespace puppet::runtime