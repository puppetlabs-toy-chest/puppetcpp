/**
 * @file
 * Declares the Puppet expression evaluator.
 */
#pragma once

#include "../ast/syntax_tree.hpp"
#include "context.hpp"
#include <cstdint>
#include <vector>
#include <exception>
#include <memory>

namespace puppet { namespace runtime {

    /**
     * Exception for evaluation errors.
     */
    struct evaluation_exception : std::runtime_error
    {
        /**
         * Constructs an evaluator exception.
         * @param position The position where evaluation failed.
         * @param message The exception message.
         */
        evaluation_exception(lexer::position position, std::string const& message);

        /**
         * Gets the position where evaluation failed.
         * @return Returns the position where evaluation failed.
         */
        lexer::position const& position() const;

     private:
        lexer::position _position;
    };

    /**
     * Represents the Puppet language expression evaluator.
     */
    struct expression_evaluator
    {
        /**
         * Constructs a expression evaluator.
         * @param context The runtime evaluation context.
         * @param tree The syntax tree being evaluated.
         */
        expression_evaluator(runtime::context& context, std::shared_ptr<ast::syntax_tree> tree);

        /**
         * Gets the current evaluation context.
         * @return Returns the current evaluation context.
         */
        runtime::context& context();

        /**
         * Gets the current evaluation context.
         * @return Returns the current evaluation context.
         */
        runtime::context const& context() const;

        /**
         * Gets the syntax tree being evaluated.
         * @return Returns the syntax tree being evaluated.
         */
        std::shared_ptr<ast::syntax_tree> const& tree() const;

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

        /**
         * Determines if a value is a "match" for an expected value.
         * Uses the match operator for expected regex values or equality for other expected values.
         * @param actual The actual value.
         * @param actual_position The actual position.
         * @param expected The expected value.
         * @param expected_position The expected position.
         * @return Returns true if the values match or false if not.
         */
        bool is_match(values::value& actual, lexer::position const& actual_position, values::value& expected, lexer::position const& expected_position);

     private:
        static bool is_productive(ast::expression const& expr);
        static bool is_productive(ast::primary_expression const& expr);

        void climb_expression(
            values::value& left,
            lexer::position& left_position,
            std::uint8_t min_precedence,
            std::vector<ast::binary_expression>::const_iterator& begin,
            std::vector<ast::binary_expression>::const_iterator const& end);

        void evaluate(
            values::value& left,
            lexer::position const& left_position,
            ast::binary_operator op,
            values::value& right,
            lexer::position& right_position);

        static uint8_t get_precedence(ast::binary_operator op);
        static bool is_right_associative(ast::binary_operator op);

        runtime::context& _context;
        std::shared_ptr<ast::syntax_tree> _tree;
    };

}}  // namespace puppet::runtime