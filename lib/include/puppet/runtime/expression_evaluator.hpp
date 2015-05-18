/**
 * @file
 * Declares the Puppet expression evaluator.
 */
#pragma once

#include "../compiler/node.hpp"
#include "../logging/logger.hpp"
#include "../ast/expression.hpp"
#include "scope.hpp"
#include "catalog.hpp"
#include <cstdint>
#include <vector>
#include <exception>
#include <functional>
#include <deque>

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
         * Constructs a expression evaluator.
         * @param logger The logger to use for logging messages.
         * @param catalog The catalog being compiled.
         * @param path The path to the file being compiled.
         * @param node The node being compiled.
         * @param warning A function to call to output a warning at a given position.
         */
        explicit expression_evaluator(logging::logger& logger, runtime::catalog& catalog, std::string const& path, compiler::node& node, std::function<void(lexer::token_position const&, std::string const&)> const& warning);

        /**
         * Gets the logger used for logging messages.
         * @return Returns the logger used for logging messages.
         */
        logging::logger& logger();

        /**
         * Gets the catalog being compiled.
         * @return Returns the catalog being compiled.
         */
        runtime::catalog& catalog();

        /**
         * Gets the path to the file being compiled.
         * @return Returns the path to the file being compiled.
         */
        std::string const& path() const;

        /**
         * Gets the current compilation node.
         * @return Returns the current compilation node.
         */
        compiler::node& node();

        /**
         * Gets the current scope.
         * @return Returns the current scope.
         */
        runtime::scope& scope();

        /**
         * Gets the top scope.
         * @return Returns the top scope.
         */
        runtime::scope& top();

        /**
         * Adds a scope with the given name.
         * Note: if a scope of the same name already exists, the existing scope is returned unmodified.
         * @param name The name of the scope to add.
         * @param scope The scope to add.
         * @return Returns the scope.
         */
        runtime::scope* add_scope(std::string name, runtime::scope scope);

        /**
         * Finds a scope by name.
         * @param name The name of the scope to find.
         * @return Returns a pointer to the scope if found or nullptr if the scope is not found.
         */
        runtime::scope* find_scope(std::string const& name);

        /**
         * Pushes the given scope.
         * @param current The new current scope.
         */
        void push_scope(runtime::scope& current);

        /**
         * Pops the current scope.
         * @return Returns true if the scope was popped or false if not (i.e. already at top scope).
         */
        bool pop_scope();

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
         * Emits a warning with the given position and message.
         * @param position The position of the warning.
         * @param message The warning message.
         */
        void warn(lexer::token_position const& position, std::string const& message) const;

     private:
        static bool is_productive(ast::expression const& expr);
        static bool is_productive(ast::primary_expression const& expr);

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

        logging::logger& _logger;
        runtime::catalog& _catalog;
        std::string const& _path;
        compiler::node& _node;
        std::unordered_map<std::string, runtime::scope> _scopes;
        std::deque<runtime::scope*> _scope_stack;
        std::function<void(lexer::token_position const&, std::string const&)> const& _warn;
    };

    /**
     * Helper for creating an ephemeral scope.
     */
    struct ephemeral_scope
    {
        /**
         * Constructs an ephemeral scope.
         * @param evaluator The current expression evaluator.
         */
        explicit ephemeral_scope(expression_evaluator& evaluator);

        /**
         * Destructs the ephemeral scope.
         */
        ~ephemeral_scope();

    private:
        expression_evaluator& _evaluator;
        scope _scope;
    };

}}  // namespace puppet::runtime