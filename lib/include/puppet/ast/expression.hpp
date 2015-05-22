/**
 * @file
 * Declares the AST expressions.
 */
#pragma once

#include "undef.hpp"
#include "defaulted.hpp"
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
#include <functional>

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
    struct postfix_expression;
    struct expression;

    /**
     * Represents a basic expression.
     */
    typedef boost::variant<
        undef,
        defaulted,
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
     * Represents a control-flow expression.
     */
    typedef boost::variant<
        boost::recursive_wrapper<case_expression>,
        boost::recursive_wrapper<if_expression>,
        boost::recursive_wrapper<unless_expression>,
        boost::recursive_wrapper<function_call_expression>
    > control_flow_expression;

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
     * Represents a primary expression.
     */
    typedef boost::variant<
        boost::blank,
        basic_expression,
        control_flow_expression,
        catalog_expression,
        boost::recursive_wrapper<unary_expression>,
        boost::recursive_wrapper<postfix_expression>,
        boost::recursive_wrapper<expression>
    > primary_expression;

    /**
     * Gets the position of the primary expression.
     * @param expr The primary expression to get the position of.
     * @return Returns the position of the primary expression.
     */
    lexer::position const& get_position(primary_expression const& expr);

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
        unary_expression(lexer::position position, unary_operator op, primary_expression operand);

        /**
         * The position of the unary expression.
         */
        lexer::position position;

        /**
         * The unary operator.
         */
        unary_operator op;

        /**
         * The operand.
         */
        primary_expression operand;
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
         * The binary operator.
         */
        binary_operator op;

        /**
         * The right-hand side of the binary expression.
         */
        primary_expression operand;

        /**
         * Gets the position of the expression.
         * @return Returns the position of the expression.
         */
        lexer::position const& position() const;
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
         * Constructs an expression.
         * @param primary The primary expression.
         * @param binary The binary expressions to apply.
         */
        explicit expression(primary_expression primary, std::vector<binary_expression> binary = std::vector<binary_expression>());

        /**
         * The primary expression.
         */
        primary_expression primary;

        /**
         * The remaining binary expressions.
         */
        std::vector<binary_expression> binary;

        /**
         * Gets the position of the expression.
         */
        lexer::position const& position() const;
    };

    /**
     * Stream insertion operator for AST expression.
     * @param os The output stream to write the expression to.
     * @param expr The expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, expression const& expr);

}}  // namespace puppet::ast

namespace std
{
    /**
     * Responsible for hashing ast::unary_operator.
     * Note: this specialization is unnecessary in C++14.
     */
    template<>
    struct hash<puppet::ast::unary_operator>
    {
        /**
         * The argument type.
         */
        typedef puppet::ast::unary_operator argument_type;
        /**
         * The underlying type of the enum.
         */
        typedef std::underlying_type<argument_type>::type underlying_type;
        /**
         * The result type of the hash.
         */
        typedef std::hash<underlying_type>::result_type result_type;

        /**
         * Responsible for hashing the argument.
         * @param arg The argument to hash.
         * @return Returns the hash value.
         */
        result_type operator()(argument_type const& arg) const
        {
            std::hash<underlying_type> hasher;
            return hasher(static_cast<underlying_type>(arg));
        }
    };

    /**
     * Responsible for hashing ast::unary_operator.
     * Note: this specialization is unnecessary in C++14.
     */
    template<>
    struct hash<puppet::ast::binary_operator>
    {
        /**
         * The argument type.
         */
        typedef puppet::ast::binary_operator argument_type;
        /**
         * The underlying type of the enum.
         */
        typedef std::underlying_type<argument_type>::type underlying_type;
        /**
         * The result type of the hash.
         */
        typedef std::hash<underlying_type>::result_type result_type;

        /**
         * Responsible for hashing the argument.
         * @param arg The argument to hash.
         * @return Returns the hash value.
         */
        result_type operator()(argument_type const& arg) const
        {
            std::hash<underlying_type> hasher;
            return hasher(static_cast<underlying_type>(arg));
        }
    };
}