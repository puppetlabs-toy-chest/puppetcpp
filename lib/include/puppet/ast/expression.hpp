/**
 * @file
 * Declares the AST expressions.
 */
#pragma once

#include "undef.hpp"
#include "boolean.hpp"
#include "number.hpp"
#include "string.hpp"
#include "regex.hpp"
#include "variable.hpp"
#include "type.hpp"
#include "name.hpp"
#include "bare_word.hpp"
#include <boost/variant.hpp>
#include <iostream>
#include <vector>

namespace puppet { namespace ast {

    // Forward declarations of recursive expressions
    struct array;
    struct hash;
    struct selector_expression;
    struct case_expression;
    struct if_expression;
    struct unless_expression;
    struct method_call_expression;
    struct function_call_expression;
    struct resource_expression;
    struct resource_defaults_expression;
    struct resource_override_expression;
    struct class_definition_expression;
    struct defined_type_expression;
    struct node_definition_expression;
    struct collection_expression;
    struct unary_expression;
    struct access_expression;
    struct expression;

    /**
     * Represents a basic expression.
     */
    typedef boost::variant<
        undef,
        boolean,
        number,
        string,
        regex,
        variable,
        name,
        bare_word,
        ast::type,
        boost::recursive_wrapper<array>,
        boost::recursive_wrapper<hash>
    > basic_expression;

    /**
     * Gets the position of the basic expression.
     * @param expr The basic expression to get the position of.
     * @return Returns the position of the basic expression.
     */
    lexer::token_position const& get_position(basic_expression const& expr);

    /**
     * Represents a control-flow expression.
     */
    typedef boost::variant<
        boost::recursive_wrapper<selector_expression>,
        boost::recursive_wrapper<case_expression>,
        boost::recursive_wrapper<if_expression>,
        boost::recursive_wrapper<unless_expression>,
        boost::recursive_wrapper<method_call_expression>,
        boost::recursive_wrapper<function_call_expression>
    > control_flow_expression;

    /**
     * Gets the position of the control flow expression.
     * @param expr The control flow expression to get the position of.
     * @return Returns the position of the control flow expression.
     */
    lexer::token_position const& get_position(control_flow_expression const& expr);

    /**
     * Represents a catalog expression.
     */
    typedef boost::variant<
        boost::recursive_wrapper<resource_expression>,
        boost::recursive_wrapper<resource_defaults_expression>,
        boost::recursive_wrapper<resource_override_expression>,
        boost::recursive_wrapper<class_definition_expression>,
        boost::recursive_wrapper<defined_type_expression>,
        boost::recursive_wrapper<node_definition_expression>,
        boost::recursive_wrapper<collection_expression>
    > catalog_expression;

    /**
     * Gets the position of the catalog expression.
     * @param expr The catalog expression to get the position of.
     * @return Returns the position of the catalog expression.
     */
    lexer::token_position const& get_position(catalog_expression const& expr);

    /**
     * Represents a primary expression.
     */
    typedef boost::variant<
        boost::blank,
        basic_expression,
        control_flow_expression,
        catalog_expression,
        boost::recursive_wrapper<unary_expression>,
        boost::recursive_wrapper<access_expression>,
        boost::recursive_wrapper<expression>
    > primary_expression;

    /**
     * Gets the position of the primary expression.
     * @param expr The primary expression to get the position of.
     * @return Returns the position of the primary expression.
     */
    lexer::token_position const& get_position(primary_expression const& expr);

    /**
     * Determines if the expression is blank.
     * @return Returns true if the expression is blank or false if not.
     */
    bool is_blank(primary_expression const& expr);

    /**
     * Represents a unary operator.
     */
    enum class unary_operator
    {
        /**
         * No operator.
         */
        none,
        /**
         * The logical not (!) operator.
         */
        logical_not,
        /**
         * The numerical negation (-) operator.
         */
        negate,
        /**
         * The splat (*) operator.
         */
        splat
    };

    /**
     * Stream insertion operator for AST unary operator.
     * @param os The output stream to write the operator to.
     * @param op The operator to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, unary_operator op);

    /**
     * Represents an AST unary expression.
     */
    struct unary_expression
    {
        /**
         * Default constructor for unary expression.
         */
        unary_expression();

        /**
         * Constructs a unary expression with the given position, operator, and primary expression.
         * @param position The position of the unary expression.
         * @param op The unary operator for the expression.
         * @param operand The primary expression operand.
         */
        unary_expression(lexer::token_position position, unary_operator op, primary_expression operand);

        /**
         * Gets the unary operator for the expression.
         * @return Returns the unary operator for the expression.
         */
        unary_operator op() const;

        /**
         * Gets the operand for the expression.
         * @return Returns the operand for the expression.
         */
        primary_expression const& operand() const;

        /**
         * Gets the position of the unary expression.
         * @return Returns the position of the unary expression.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        unary_operator _op;
        primary_expression _operand;
    };

    /**
     * Stream insertion operator for AST unary expression.
     * @param os The output stream to write the expression to.
     * @param expr The expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, unary_expression const& expr);

    /**
     * Represents a binary operator.
     */
    enum class binary_operator
    {
        /**
         * No operator.
         */
        none,
        /**
         * The "in" operator.
         */
        in,
        /**
         * The match (=~) operator.
         */
        match,
        /**
         * The not match (!~) operator.
         */
        not_match,
        /**
         * The multiply (*) operator.
         */
        multiply,
        /**
         * The divide (/) operator.
         */
        divide,
        /**
         * The modulo (%) operator.
         */
        modulo,
        /**
         * The plus (+) operator.
         */
        plus,
        /**
         * The minus (-) operator.
         */
        minus,
        /**
         * The left shift (<<) operator.
         */
        left_shift,
        /**
         * The right shift (>>) operator.
         */
        right_shift,
        /**
         * The equals (==) operator.
         */
        equals,
        /**
         * The not equals (!=) operator.
         */
        not_equals,
        /**
         * The greater than (>) operator.
         */
        greater_than,
        /**
         * The greater than or equal to (>=) operator.
         */
        greater_equals,
        /**
         * The less than (<) operator.
         */
        less_than,
        /**
         * The less than or equals to operator (<=) operator.
         */
        less_equals,
        /**
         * The logical and (and) operator.
         */
        logical_and,
        /**
         * The logical or (or) operator.
         */
        logical_or,
        /**
         * The assignment (=) operator.
         */
        assignment,
        /**
         * The in edge (->) operator.
         */
        in_edge,
        /**
         * The in edge with subscription (~>) operator.
         */
        in_edge_subscribe,
        /**
         * The out edge (<-) operator.
         */
        out_edge,
        /**
         * The out edge with subscription (<~) operator.
         */
        out_edge_subscribe
    };

    /**
     * Stream insertion operator for AST binary operator.
     * @param os The output stream to write the operator to.
     * @param op The operator to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, binary_operator op);

    /**
     * Represents an AST binary expression.
     */
    struct binary_expression
    {
        /**
         * Default constructor for binary_expression.
         */
        binary_expression();

        /**
         * Constructs a binary_expression with the given operator and right-hand side.
         * @param op The binary operator used in the expression.
         * @param operand The right-hand primary expression operand.
         */
        binary_expression(binary_operator op, primary_expression operand);

        /**
         * Gets the binary operator in the expression.
         * @return Returns the binary operator in the expression.
         */
        binary_operator op() const;

        /**
         * Gets the right-hand operand of the binary expression.
         * @return Returns the right-hand operand of the binary expression.
         */
        primary_expression const& operand() const;

        /**
         * Gets the position of the binary expression.
         * @return Returns the position of the binary expression.
         */
        lexer::token_position const& position() const;

     private:
        binary_operator _op;
        primary_expression _operand;
    };

    /**
     * Stream insertion operator for AST binary expression.
     * @param os The output stream to write the expression to.
     * @param expr The expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, binary_expression const& expr);

    /**
     * Represents an AST expression.
     */
    struct expression
    {
        /**
         * Default constructor for expression.
         */
        expression();

        /**
         * Constructs an expression with the first primary expression and a variable number of binary expressions.
         * @param first The first primary expression in the expression.
         * @param remainder The remaining binary expressions; empty if a unary expression.
         */
        expression(primary_expression first, std::vector<binary_expression> remainder = std::vector<binary_expression>());

        /**
         * Gets the first primary expression in the expression.
         * @return Returns the first primary expression in the expression.
         */
        primary_expression const& first() const;

        /**
         * Gets the remainder of the expression (for binary expressions).
         * Empty for unary expressions.
         * @return Returns the remainder of the expression.
         */
        std::vector<binary_expression> const& remainder() const;

        /**
         * Gets the position of the expression.
         * @return Returns the position of the expression.
         */
        lexer::token_position const& position() const;

        /**
         * Determines if the expression is blank.
         * @return Returns true if the expression is blank or false if not.
         */
        bool blank() const;

     private:
        primary_expression _first;
        std::vector<binary_expression> _remainder;
    };

    /**
     * Stream insertion operator for AST expression.
     * @param os The output stream to write the expression to.
     * @param expr The expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, expression const& expr);

}}  // namespace puppet::ast
