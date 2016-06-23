/**
 * @file
 * Declares the AST structures.
 */
#pragma once

#include "../lexer/tokens.hpp"
#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <memory>

namespace puppet { namespace compiler {

    // Forward declaration of module.
    struct module;

}}

namespace puppet { namespace compiler { namespace ast {

    // Forward declaration of syntax tree
    struct syntax_tree;

    /**
     * Represents AST context.
     * This is primarily used for error reporting.
     * AST nodes either derive from context or provide a context() member function to return their context.
     */
    struct context
    {
        /**
         * Stores the beginning position.
         */
        lexer::position begin;

        /**
         * Stores the ending position.
         */
        lexer::position end;

        /**
         * Stores the back pointer to the root of the tree.
         */
        syntax_tree* tree = nullptr;
    };

    /**
     * Equality operator for context.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two contexts are equal or false if not.
     */
    bool operator==(context const& left, context const& right);

    /**
     * Inequality operator for context.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two contexts are not equal or false if they are equal.
     */
    bool operator!=(context const& left, context const& right);

    /**
     * Represents a literal undef.
     */
    struct undef : context
    {
    };

    /**
     * Stream insertion operator for undef.
     * @param os The output stream to write to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, undef const&);

    /**
     * Represents a literal default.
     */
    struct defaulted : context
    {
    };

    /**
     * Stream insertion operator for defaulted.
     * @param os The output stream to write to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, defaulted const&);

    /**
     * Represents a literal boolean.
     */
    struct boolean : context
    {
        /**
         * Stores the value of the literal boolean.
         */
        bool value = false;
    };

    /**
     * Equality operator for boolean.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two booleans are equal or false if not.
     */
    bool operator==(boolean const& left, boolean const& right);

    /**
     * Inequality operator for boolean.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two booleans are not equal or false if they are equal.
     */
    bool operator!=(boolean const& left, boolean const& right);

    /**
     * Stream insertion operator for boolean.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, boolean const& node);

    /**
     * Equality operator for boolean.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two booleans are equal or false if not.
     */
    bool operator==(boolean const& left, boolean const& right);

    /**
     * Inequality operator for boolean.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two booleans are not equal or false if they are equal.
     */
    bool operator!=(boolean const& left, boolean const& right);

    /**
     * Represents a literal number.
     */
    struct number : context
    {
        /**
         * The number value type.
         */
        using value_type = lexer::number_token::value_type;

        /**
         * Stores the base of the number.
         */
        lexer::numeric_base base = lexer::numeric_base::decimal;

        /**
         * Stores the value of the literal number.
         */
        value_type value = value_type();
    };

    /**
     * Stream insertion operator for number.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, number const& node);

    /**
     * Equality operator for number.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two numbers are equal or false if not.
     */
    bool operator==(number const& left, number const& right);

    /**
     * Inequality operator for number.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two numbers are not equal or false if they are equal.
     */
    bool operator!=(number const& left, number const& right);

    /**
     * Represents a literal string.
     */
    struct string : context
    {
        /**
         * Stores the data format of the string (heredocs only).
         */
        std::string format;

        /**
         * Stores the value of the literal string.
         */
        std::string value;

        /**
         * Stores the string's margin (heredoc only).
         */
        size_t margin = 0;
    };

    /**
     * Stream insertion operator for string.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, string const& node);

    /**
     * Equality operator for string.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two strings are equal or false if not.
     */
    bool operator==(string const& left, string const& right);

    /**
     * Inequality operator for string.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two strings are not equal or false if they are equal.
     */
    bool operator!=(string const& left, string const& right);

    /**
     * Represents a literal regex.
     */
    struct regex : context
    {
        /**
         * Stores the value of the literal regex.
         */
        std::string value;
    };

    /**
     * Stream insertion operator for regex.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, regex const& node);

    /**
     * Equality operator for regex.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two regexes are equal or false if not.
     */
    bool operator==(regex const& left, regex const& right);

    /**
     * Inequality operator for regex.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two regexes are not equal or false if they are equal.
     */
    bool operator!=(regex const& left, regex const& right);

    /**
     * Represents a variable.
     */
    struct variable : context
    {
        /**
         * Stores the name of the variable.
         */
        std::string name;
    };

    /**
     * Stream insertion operator for variable.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, variable const& node);

    /**
     * Equality operator for variable.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two variables are equal or false if not.
     */
    bool operator==(variable const& left, variable const& right);

    /**
     * Inequality operator for variable.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two variables are not equal or false if they are equal.
     */
    bool operator!=(variable const& left, variable const& right);

    /**
     * Represents a name.
     */
    struct name : context
    {
        /**
         * Stores the value of the name.
         */
        std::string value;
    };

    /**
     * Stream insertion operator for name.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, name const& node);

    /**
     * Equality operator for name.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two names are equal or false if not.
     */
    bool operator==(name const& left, name const& right);

    /**
     * Inequality operator for name.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two names are not equal or false if they are equal.
     */
    bool operator!=(name const& left, name const& right);

    /**
     * Represents a bare word.
     */
    struct bare_word : context
    {
        /**
         * Stores the value of the bare word.
         */
        std::string value;
    };

    /**
     * Equality operator for bare word.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two bare words are equal or false if not.
     */
    bool operator==(bare_word const& left, bare_word const& right);

    /**
     * Inequality operator for bare word.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two bare words are not equal or false if they are equal.
     */
    bool operator!=(bare_word const& left, bare_word const& right);

    /**
     * Stream insertion operator for bare word.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, bare_word const& node);

    /**
     * Represents a type.
     */
    struct type : context
    {
        /**
         * Stores the name of the type.
         */
        std::string name;
    };

    /**
     * Stream insertion operator for type.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, type const& node);

    /**
     * Equality operator for type.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two types are equal or false if not.
     */
    bool operator==(type const& left, type const& right);

    /**
     * Inequality operator for type.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two types are not equal or false if they are equal.
     */
    bool operator!=(type const& left, type const& right);

    // Forward declarations of recursive expressions
    struct interpolated_string;
    struct array;
    struct hash;
    struct case_expression;
    struct if_expression;
    struct unless_expression;
    struct function_call_expression;
    struct new_expression;
    struct epp_render_expression;
    struct epp_render_block;
    struct epp_render_string;
    struct unary_expression;
    struct nested_expression;

    /**
     * Represents a basic expression.
     */
    struct basic_expression : boost::spirit::x3::variant<
        undef,
        defaulted,
        boolean,
        number,
        string,
        regex,
        variable,
        name,
        bare_word,
        type,
        boost::spirit::x3::forward_ast<interpolated_string>,
        boost::spirit::x3::forward_ast<array>,
        boost::spirit::x3::forward_ast<hash>,
        boost::spirit::x3::forward_ast<case_expression>,
        boost::spirit::x3::forward_ast<if_expression>,
        boost::spirit::x3::forward_ast<unless_expression>,
        boost::spirit::x3::forward_ast<function_call_expression>,
        boost::spirit::x3::forward_ast<new_expression>,
        boost::spirit::x3::forward_ast<epp_render_expression>,
        boost::spirit::x3::forward_ast<epp_render_block>,
        boost::spirit::x3::forward_ast<epp_render_string>,
        boost::spirit::x3::forward_ast<unary_expression>,
        boost::spirit::x3::forward_ast<nested_expression>
        >
    {
        // Use the base's construction and assignment semantics
        using base_type::base_type;
        using base_type::operator=;

        /**
         * Default constructor for basic expression.
         */
        basic_expression() = default;

        /**
         * Default copy constructor for basic expression.
         */
        basic_expression(basic_expression const&) = default;

        /**
         * Default move constructor for basic expression.
         */
        basic_expression(basic_expression&&) = default;

        /**
         * Default copy assignment operator for basic expression.
         * @return Returns this basic expression.
         */
        basic_expression& operator=(basic_expression const&) = default;

        /**
         * Default move assignment operator for basic expression.
         * @return Returns this basic expression.
         */
        basic_expression& operator=(basic_expression&&) = default;

        /**
         * Gets the context of the basic expression.
         * @return Returns the context of the basic expression.
         */
        ast::context context() const;

        /**
         * Determines if the expression is a splat.
         * @return Returns true if the expression is a splat or false if not.
         */
        bool is_splat() const;

        /**
         * Determines if the expression is 'default'.
         * @return Returns true if the expression is default or false if not.
         */
        bool is_default() const;
    };

    /**
     * Stream insertion operator for basic expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, basic_expression const& node);

    // Forward declarations of recursive postfix expressions
    struct selector_expression;
    struct access_expression;
    struct method_call_expression;

    /**
     * Represents a postfix operation.
     */
    struct postfix_operation : boost::spirit::x3::variant<
        boost::spirit::x3::forward_ast<selector_expression>,
        boost::spirit::x3::forward_ast<access_expression>,
        boost::spirit::x3::forward_ast<method_call_expression>
        >
    {
        // Use the base's construction and assignment semantics
        using base_type::base_type;
        using base_type::operator=;

        /**
         * Default constructor for postfix operation.
         */
        postfix_operation() = default;

        /**
         * Default copy constructor for postfix operation.
         */
        postfix_operation(postfix_operation const&) = default;

        /**
         * Default move constructor for postfix operation.
         */
        postfix_operation(postfix_operation&&) = default;

        /**
         * Default copy assignment operator for postfix operation.
         * @return Returns this postfix operation.
         */
        postfix_operation& operator=(postfix_operation const&) = default;

        /**
         * Default move assignment operator for postfix operation.
         * @return Returns this postfix operation.
         */
        postfix_operation& operator=(postfix_operation&&) = default;

        /**
         * Gets the context of the postfix operation.
         * @return Returns the context of the postfix operation.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for postfix operation.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, postfix_operation const& node);

    /**
     * Represents a postfix expression.
     */
    struct postfix_expression
    {
        /**
         * Stores the postfix operand.
         */
        basic_expression operand;

        /**
         * Stores the optional postfix operations.
         */
        std::vector<postfix_operation> operations;

        /**
         * Gets the context of the postfix expression.
         * @return Returns the context of the postfix expression.
         */
        ast::context context() const;

        /**
         * Validates the postfix expression as a type specification.
         * Throws parse exceptions if validation fails.
         */
        void validate_type() const;

        /**
         * Determines if the expression is a splat.
         * @return Returns true if the expression is a splat or false if not.
         */
        bool is_splat() const;

        /**
         * Determines if the expression is 'default'.
         * @return Returns true if the expression is default or false if not.
         */
        bool is_default() const;
    };

    /**
     * Stream insertion operator for postfix expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, postfix_expression const& node);

    /**
     * Represents a binary operator.
     */
    enum class binary_operator
    {
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
        assignment
    };

    /**
     * Stream insertion operator for binary operator.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, binary_operator const& node);

    /**
     * Gets the low-to-high operator precedence of the given binary operator.
     * @param op The operator to get the precedence for.
     * @return Returns the low-to-high operator precedence of the given binary operator.
     */
    unsigned int precedence(binary_operator op);

    /**
     * Determines if the given binary operator is right-associative.
     * @param op The operator to check for right-associativity.
     * @return Returns true if the operator is right-associative or false if not.
     */
    bool is_right_associative(binary_operator op);

    /**
     * Hashes a binary operator.
     * @param op The operator to hash.
     * @return Returns the hash value for the binary operator.
     */
    size_t hash_value(binary_operator op);

    /**
     * Represents a binary operation.
     * Stores the operator and the RHS of the expression.
     */
    struct binary_operation
    {
        /**
         * Stores the position of the operator.
         */
        lexer::position operator_position;

        /**
         * Stores the binary operator.
         */
        binary_operator operator_ = static_cast<binary_operator>(0);

        /**
         * Stores the operand expression.
         */
        postfix_expression operand;

        /**
         * Gets the context of the binary operation.
         * @return Returns the context of the binary operation.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for binary operation.
     * @param os The output stream to write to.
     * @param operation The operation to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, binary_operation const& operation);

    /**
     * Represents an expression.
     */
    struct expression
    {
        /**
         * Stores the first operand in the expression.
         */
        postfix_expression operand;

        /**
         * Stores the binary operations of the expression.
         */
        std::vector<binary_operation> operations;

        /**
         * Gets the context of the expression.
         * @return Returns the context of the expression.
         */
        ast::context context() const;

        /**
         * Determines if the expression is a splat.
         * @return Returns true if the expression is a splat or false if not.
         */
        bool is_splat() const;

        /**
         * Determines if the expression is 'default'.
         * @return Returns true if the expression is default or false if not.
         */
        bool is_default() const;
    };

    /**
     * Stream insertion operator for expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, expression const& node);

    // Forward declarations of recursive statements
    struct class_statement;
    struct defined_type_statement;
    struct node_statement;
    struct function_statement;
    struct produces_statement;
    struct consumes_statement;
    struct application_statement;
    struct site_statement;
    struct type_alias_statement;
    struct function_call_statement;
    struct relationship_statement;

    /**
     * Represents a Puppet statement.
     */
    struct statement : boost::spirit::x3::variant<
        boost::spirit::x3::forward_ast<class_statement>,
        boost::spirit::x3::forward_ast<defined_type_statement>,
        boost::spirit::x3::forward_ast<node_statement>,
        boost::spirit::x3::forward_ast<function_statement>,
        boost::spirit::x3::forward_ast<produces_statement>,
        boost::spirit::x3::forward_ast<consumes_statement>,
        boost::spirit::x3::forward_ast<application_statement>,
        boost::spirit::x3::forward_ast<site_statement>,
        boost::spirit::x3::forward_ast<type_alias_statement>,
        boost::spirit::x3::forward_ast<function_call_statement>,
        boost::spirit::x3::forward_ast<relationship_statement>
        >
    {
        // Use the base's construction and assignment semantics
        using base_type::base_type;
        using base_type::operator=;

        /**
         * Default constructor for statement.
         */
        statement() = default;

        /**
         * Default copy constructor for statement.
         */
        statement(statement const&) = default;

        /**
         * Default move constructor for statement.
         */
        statement(statement&&) = default;

        /**
         * Default copy assignment operator for statement.
         * @return Returns this statement.
         */
        statement& operator=(statement const&) = default;

        /**
         * Default move assignment operator for statement.
         * @return Returns this statement.
         */
        statement& operator=(statement&&) = default;

        /**
         * Gets the context of the statement.
         * @return Returns the context of the statement.
         */
        ast::context context() const;

        /**
         * Validates the statement.
         * Throws parse exceptions if validation fails.
         * @param effective True if the statement is required to be effective or false if not.
         */
        void validate(bool effective = false) const;
    };

    /**
     * Stream insertion operator for statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, statement const& node);

    /**
     * Represents literal string text in an interpolated string.
     */
    struct literal_string_text : context
    {
        /**
         * Stores the literal string text to render.
         */
        std::string text;
    };

    /**
     * Stream insertion operator for literal string text.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, literal_string_text const& node);

    /**
     * Equality operator for literal string text.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two strings are equal or false if not.
     */
    bool operator==(literal_string_text const& left, literal_string_text const& right);

    /**
     * Inequality operator for literal string text.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two strings are not equal or false if they are equal.
     */
    bool operator!=(literal_string_text const& left, literal_string_text const& right);

    /**
     * Represents part of an interpolated string.
     */
    struct interpolated_string_part : boost::spirit::x3::variant<
        literal_string_text,
        variable,
        boost::spirit::x3::forward_ast<expression>
    >
    {
        // Use the base's construction and assignment semantics
        using base_type::base_type;
        using base_type::operator=;

        /**
         * Default constructor for interpolated string part.
         */
        interpolated_string_part() = default;

        /**
         * Default copy constructor for interpolated string part.
         */
        interpolated_string_part(interpolated_string_part const&) = default;

        /**
         * Default move constructor for interpolated string part.
         */
        interpolated_string_part(interpolated_string_part&&) = default;

        /**
         * Default copy assignment operator for interpolated string part.
         * @return Returns this interpolated string part.
         */
        interpolated_string_part& operator=(interpolated_string_part const&) = default;

        /**
         * Default move assignment operator for interpolated string part.
         * @return Returns this interpolated string part.
         */
        interpolated_string_part& operator=(interpolated_string_part&&) = default;

        /**
         * Gets the context of the interpolated string part.
         * @return Returns the context of the interpolated string part.
         */
        ast::context context() const;
    };

    /**
     * Represents an interpolated string.
     */
    struct interpolated_string : context
    {
        /**
         * Stores the data format of the string (heredocs only).
         */
        std::string format;

        /**
         * Stores the parts that comprised the interpolated string.
         */
        std::vector<interpolated_string_part> parts;

        /**
         * Stores the string's margin (heredoc only).
         */
        size_t margin = 0;
    };

    /**
     * Stream insertion operator for interpolated string.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, interpolated_string const& node);

    /**
     * Equality operator for interpolated string.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two strings are equal or false if not.
     */
    bool operator==(interpolated_string const& left, interpolated_string const& right);

    /**
     * Inequality operator for interpolated string.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two strings are not equal or false if they are equal.
     */
    bool operator!=(interpolated_string const& left, interpolated_string const& right);

    /**
     * Represents an array literal.
     */
    struct array : context
    {
        /**
         * Stores the array elements.
         */
        std::vector<expression> elements;
    };

    /**
     * Stream insertion operator for array.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, array const& node);

    /**
     * The pair type.
     */
    using pair = std::pair<expression, expression>;

    /**
     * Represents a hash literal.
     */
    struct hash : context
    {
        /**
         * Stores the hash elements.
         */
        std::vector<pair> elements;
    };

    /**
     * Stream insertion operator for pair.
     * @param os The output stream to write to.
     * @param pair The pair to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::pair const& pair);

    /**
     * Stream insertion operator for hash.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, hash const& node);

    /**
     * Represents a case proposition.
     */
    struct proposition
    {
        /**
         * Stores the options.
         */
        std::vector<expression> options;

        /**
         * Stores the body.
         */
        std::vector<statement> body;

        /**
         * Stores the ending position.
         */
        lexer::position end;
    };

    /**
     * Stream insertion operator for proposition.
     * @param os The output stream to write to.
     * @param proposition The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::proposition const& proposition);

    /**
     * Represents a case expression.
     */
    struct case_expression : context
    {
        /**
         * Stores the conditional expression.
         */
        ast::expression conditional;

        /**
         * Stores the case propositions.
         */
        std::vector<proposition> propositions;
    };

    /**
     * Stream insertion operator for case expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, case_expression const& node);

    /**
     * Represents an else.
     */
    struct else_
    {
        /**
         * Stores the beginning position.
         */
        lexer::position begin;

        /**
         * Stores the body.
         */
        std::vector<statement> body;

        /**
         * Stores the ending position.
         */
        lexer::position end;
    };

    /**
     * Stream insertion operator for an else.
     * @param os The output stream to write to.
     * @param else_ The else to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::else_ const& else_);

    /**
     * Represents an else-if.
     */
    struct elsif
    {
        /**
         * Stores the beginning position.
         */
        lexer::position begin;

        /**
         * Stores the conditional.
         */
        expression conditional;

        /**
         * Stores the body.
         */
        std::vector<statement> body;

        /**
         * Stores the ending position.
         */
        lexer::position end;
    };

    /**
     * Stream insertion operator for an elsif.
     * @param os The output stream to write to.
     * @param elsif The elsif to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::elsif const& elsif);

    /**
     * Represents an if expression.
     */
    struct if_expression
    {
        /**
         * Stores the beginning position.
         */
        lexer::position begin;

        /**
         * Stores the conditional.
         */
        expression conditional;

        /**
         * Stores the body.
         */
        std::vector<statement> body;

        /**
         * Stores the ending position.
         */
        lexer::position end;

        /**
         * Stores the "else-ifs".
         */
        std::vector<elsif> elsifs;

        /**
         * Stores the optional "else".
         */
        boost::optional<ast::else_> else_;

        /**
         * Gets the context of the if expression.
         * @return Returns the context of the if expression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for if expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, if_expression const& node);

    /**
     * Represents an unless expression.
     */
    struct unless_expression
    {
        /**
         * Stores the beginning position.
         */
        lexer::position begin;

        /**
         * Stores the conditional.
         */
        expression conditional;

        /**
         * Stores the body.
         */
        std::vector<statement> body;

        /**
         * Stores the ending position.
         */
        lexer::position end;

        /**
         * Stores the optional else.
         */
        boost::optional<ast::else_> else_;

        /**
         * Gets the context of the unless expression.
         * @return Returns the context of the unless expression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for unless expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, unless_expression const& node);

    /**
     * Represents a function or lambda parameter.
     */
    struct parameter
    {
        /**
         * Stores the optional type expression.
         */
        boost::optional<postfix_expression> type;

        /**
         * Stores the optional position of the "captures all" specifier.
         */
        boost::optional<lexer::position> captures;

        /**
         * Stores the parameter's variable.
         */
        ast::variable variable;

        /**
         * Stores the default value expression.
         */
        boost::optional<expression> default_value;

        /**
         * Gets the context of the parameter.
         * @return Returns the context of the parameter.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for parameter.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, parameter const& node);

    /**
     * Represents a lambda expression.
     */
    struct lambda_expression : context
    {
        /**
         * Stores the parameters.
         */
        std::vector<parameter> parameters;

        /**
         * Stores the body.
         */
        std::vector<statement> body;
    };

    /**
     * Stream insertion operator for lambda expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, lambda_expression const& node);

    /**
     * Represents a function call expression.
     */
    struct function_call_expression
    {
        /**
         * Stores the name of the function.
         */
        name function;

        /**
         * Stores the arguments.
         */
        std::vector<expression> arguments;

        /**
         * Stores the optional ending position.
         */
        boost::optional<lexer::position> end;

        /**
         * Stores the optional lambda.
         */
        boost::optional<lambda_expression> lambda;

        /**
         * Gets the context of the function call expression.
         * @return Returns the context of the function call expression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for function call expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, function_call_expression const& node);

    /**
     * Represents a new expression.
     */
    struct new_expression
    {
        /**
         * Stores the type postfix expression.
         */
        postfix_expression type;

        /**
         * Stores the arguments to new.
         */
        std::vector<expression> arguments;

        /**
         * Stores the ending position.
         */
        lexer::position end;

        /**
         * Stores the optional lambda.
         */
        boost::optional<lambda_expression> lambda;

        /**
         * Gets the context of the function call expression.
         * @return Returns the context of the function call expression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for new expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, new_expression const& node);

    /**
     * Represents an EPP render expression.
     */
    struct epp_render_expression : context
    {
        /**
         * Stores the expression to render.
         */
        ast::expression expression;
    };

    /**
     * Stream insertion operator for EPP render expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, epp_render_expression const& node);

    /**
     * Represents an EPP render block expression.
     */
    struct epp_render_block : context
    {
        /**
         * Stores the block to render.
         */
        std::vector<ast::expression> block;
    };

    /**
     * Stream insertion operator for EPP render block.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, epp_render_block const& node);

    /**
     * Represents an EPP render string.
     */
    struct epp_render_string : context
    {
        /**
         * Stores the string to render.
         */
        std::string string;
    };

    /**
     * Stream insertion operator for EPP render string.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, epp_render_string const& node);

    /**
     * Represents a unary operator.
     */
    enum class unary_operator
    {
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
     * Stream insertion operator for unary operator.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, unary_operator const& node);

    /**
     * Hashes a unary operator.
     * @param oper The operator to hash.
     * @return Returns the hash value for the unary operator.
     */
    size_t hash_value(unary_operator const& oper);

    /**
     * Represents a unary expression.
     */
    struct unary_expression
    {
        /**
         * Stores the position of the operator.
         */
        lexer::position operator_position;

        /**
         * Stores the unary operator.
         */
        unary_operator operator_ = static_cast<unary_operator>(0);

        /**
         * Stores the operand expression.
         */
        postfix_expression operand;

        /**
         * Gets the context of the unary expression.
         * @return Returns the context of the unary expression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for unary expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, unary_expression const& node);

    /**
     * Represents a nested expression.
     */
    struct nested_expression : context
    {
        /**
         * Stores the expression that was nested.
         */
        ast::expression expression;
    };

    /**
     * Stream insertion operator for nested expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, nested_expression const& node);

    /**
     * Represents a selector expression.
     */
    struct selector_expression : context
    {
        /**
         * Stores the selector cases.
         */
        std::vector<pair> cases;
    };

    /**
     * Stream insertion operator for selector expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, selector_expression const& node);

    /**
     * Represents an access expression.
     */
    struct access_expression : context
    {
        /**
         * Stores the argument expressions.
         */
        std::vector<expression> arguments;
    };

    /**
     * Stream insertion operator for access expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, access_expression const& node);

    /**
     * Represents a method call expression.
     */
    struct method_call_expression
    {
        /**
         * Stores the beginning position.
         */
        lexer::position begin;

        /**
         * Stores the name of the method.
         */
        name method;

        /**
         * Stores the arguments.
         */
        std::vector<expression> arguments;

        /**
         * Stores the optional ending position.
         */
        boost::optional<lexer::position> end;

        /**
         * Stores the optional lambda.
         */
        boost::optional<lambda_expression> lambda;

        /**
         * Gets the context of the method call expression.
         * @return Returns the context of the method call expression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for method call expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, method_call_expression const& node);

    /**
     * Represents a class statement.
     */
    struct class_statement : context
    {
        /**
         * Stores the class name.
         */
        ast::name name;

        /**
         * Stores the parameters.
         */
        std::vector<parameter> parameters;

        /**
         * Stores the optional parent class name.
         */
        boost::optional<ast::name> parent;

        /**
         * Stores the body.
         */
        std::vector<statement> body;
    };

    /**
     * Stream insertion operator for class statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, class_statement const& node);

    /**
     * Represents a defined type statement.
     */
    struct defined_type_statement : context
    {
        /**
         * Stores the defined type name.
         */
        ast::name name;

        /**
         * Stores the parameters.
         */
        std::vector<parameter> parameters;

        /**
         * Stores the body.
         */
        std::vector<statement> body;
    };

    /**
     * Stream insertion operator for defined type statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, defined_type_statement const& node);

    /**
     * Represents a vector of hostname parts.
     */
    using hostname_parts = std::vector<boost::variant<name, bare_word, number>>;

    /**
     * Represents a node hostname.
     */
    struct hostname : boost::spirit::x3::variant<
            defaulted,
            string,
            regex,
            hostname_parts
        >
    {
        // Use the base's construction and assignment semantics
        using base_type::base_type;
        using base_type::operator=;

        /**
         * Default constructor for hostname.
         */
        hostname() = default;

        /**
         * Default copy constructor for hostname.
         */
        hostname(hostname const&) = default;

        /**
         * Default move constructor for hostname.
         */
        hostname(hostname&&) = default;

        /**
         * Default copy assignment operator for hostname.
         * @return Returns this hostname.
         */
        hostname& operator=(hostname const&) = default;

        /**
         * Default move assignment operator for hostname.
         * @return Returns this hostname.
         */
        hostname& operator=(hostname&&) = default;

        /**
         * Gets the context of the hostname.
         * @return Returns the context of the hostname.
         */
        ast::context context() const;

        /**
         * Determines if the hostname is the default keyword.
         * @return Returns true if the hostname is defaulted or false if not.
         */
        bool is_default() const;

        /**
         * Determines if the hostname is a regex.
         * @return Returns true if the hostname is a regex or false if not.
         */
        bool is_regex() const;

        /**
         * Determines if the hostname is valid.
         * @return Returns true if the hostname is valid or false if not.
         */
        bool is_valid() const;

        /**
         * Converts the hostname to a string.
         * @return Returns the hostname as a string.
         */
        std::string to_string() const;
    };

    /**
     * Stream insertion operator for hsotname.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, hostname const& node);

    /**
     * Represents a node statement.
     */
    struct node_statement : context
    {
        /**
         * Stores the hostnames.
         */
        std::vector<hostname> hostnames;

        /**
         * Stores the body.
         */
        std::vector<statement> body;
    };

    /**
     * Stream insertion operator for node statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, node_statement const& node);

    /**
     * Represents a statement for defining a function in the Puppet language.
     */
    struct function_statement : context
    {
        /**
         * Stores whether or not the function is private to a module.
         */
        bool is_private = false;

        /**
         * Stores the function's name.
         */
        ast::name name;

        /**
         * Stores the function's parameters.
         */
        std::vector<parameter> parameters;

        /**
         * Stores the function's body.
         */
        std::vector<statement> body;
    };

    /**
     * Stream insertion operator for function statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, function_statement const& node);

    /**
     * Represents a resource attribute operator.
     */
    enum class attribute_operator
    {
        /**
         * The assignment (=>) operator.
         */
        assignment,
        /**
         * The append (+>) operator.
         */
        append
    };

    /**
     * Stream insertion operator for attribute operator.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, attribute_operator const& node);

    /**
     * Represents a resource attribute operation.
     */
    struct attribute_operation
    {
        /**
         * Stores the attribute name.
         */
        ast::name name;

        /**
         * Stores the position of the operator.
         */
        lexer::position operator_position;

        /**
         * Stores the attribute operator.
         */
        attribute_operator operator_ = static_cast<attribute_operator>(0);

        /**
         * Stores the value expression.
         */
        expression value;

        /**
         * Gets the context of the attribute operation.
         * @return Returns the context of the attribute operation .
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for attribute operation.
     * @param os The output stream to write to.
     * @param operation The operation to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, attribute_operation const& operation);

    /**
     * Represents a produces statement.
     */
    struct produces_statement
    {
        /**
         * Stores the resource type that produces the capability type.
         */
        ast::type resource;

        /**
         * Stores the capability type being produced.
         */
        ast::type capability;

        /**
         * Stores the attribute operations.
         */
        std::vector<attribute_operation> operations;

        /**
         * Stores the ending position of the statement.
         */
        lexer::position end;

        /**
         * Gets the context of the produces statement.
         * @return Returns the context of the produces statement.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for produces statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, produces_statement const& node);

    /**
     * Represents a consumes statement.
     */
    struct consumes_statement
    {
        /**
         * Stores the resource type consuming the capability type.
         */
        ast::type resource;

        /**
         * Stores the capability type being consumed.
         */
        ast::type capability;

        /**
         * Stores the attribute operations.
         */
        std::vector<attribute_operation> operations;

        /**
         * Stores the ending position of the statement.
         */
        lexer::position end;

        /**
         * Gets the context of the consumes statement.
         * @return Returns the context of the consumes statement.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for consumes statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, consumes_statement const& node);

    /**
     * Represents an application statement.
     */
    struct application_statement : context
    {
        /**
         * Stores the application name.
         */
        ast::name name;

        /**
         * Stores the parameters.
         */
        std::vector<parameter> parameters;

        /**
         * Stores the body.
         */
        std::vector<statement> body;
    };

    /**
     * Stream insertion operator for application statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, application_statement const& node);

    /**
     * Represents a site statement.
     */
    struct site_statement : context
    {
        /**
         * Stores the body.
         */
        std::vector<statement> body;
    };

    /**
     * Stream insertion operator for site statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, site_statement const& node);

    /**
     * Represents a type alias statement.
     */
    struct type_alias_statement
    {
        /**
         * Stores the beginning position of the statement.
         */
        lexer::position begin;

        /**
         * Stores the alias type.
         */
        ast::type alias;

        /**
         * Stores the postfix expression for the type being aliased.
         */
        postfix_expression type;

        /**
         * Gets the context of the consumes statement.
         * @return Returns the context of the consumes statement.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for type alias statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, type_alias_statement const& node);

    /**
     * Represents a function call statement.
     */
    struct function_call_statement
    {
        /**
         * Stores the name of the function.
         */
        name function;

        /**
         * Stores the arguments.
         */
        std::vector<expression> arguments;

        /**
         * Stores the optional lambda.
         */
        boost::optional<lambda_expression> lambda;

        /**
         * Gets the context of the function call statement.
         * @return Returns the context of the function call statement.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for function call statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, function_call_statement const& node);

    /**
     * Represents the status of a resource.
     */
    enum class resource_status
    {
        /**
         * The resource is realized.
         */
        realized,
        /**
         * The resource is virtualized.
         */
        virtualized,
        /**
         * The resource is exported.
         */
        exported
    };

    /**
     * Stream insertion operator for resource status.
     * @param os The output stream to write to.
     * @param status The status to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource_status status);

    /**
     * Represents a resource body.
     */
    struct resource_body
    {
        /**
         * Stores the resource title.
         */
        expression title;

        /**
         * Stores the resource attribute operations.
         */
        std::vector<attribute_operation> operations;

        /**
         * Gets the context of the resource body.
         * @return Returns the context of the resource body.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for resource body.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource_body const& node);

    /**
     * Represents a resource declaration expression.
     */
    struct resource_declaration_expression : context
    {
        /**
         * Stores the resource status.
         */
        resource_status status = resource_status::realized;

        /**
         * Stores the resource type.
         */
        postfix_expression type;

        /**
         * Stores the resource bodies.
         */
        std::vector<resource_body> bodies;
    };

    /**
     * Stream insertion operator for resource declaration expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource_declaration_expression const& node);

    /**
     * Represents a resource defaults expression.
     */
    struct resource_defaults_expression : context
    {
        /**
         * Stores the resource type.
         */
        ast::type type;

        /**
         * Stores the attribute operations.
         */
        std::vector<attribute_operation> operations;
    };

    /**
     * Stream insertion operator for resource defaults expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource_defaults_expression const& node);

    /**
     * Represents the possible query operators.
     */
    enum class query_operator
    {
        /**
         * The equals (==) operator.
         */
        equals,

        /**
         * The not equals (!=) operator.
         */
        not_equals
    };

    /**
     * Stream insertion operator for query operator.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, query_operator const& node);

    /**
     * Represents an collector attribute query.
     */
    struct attribute_query
    {
        /**
         * Stores the attribute name.
         */
        name attribute;

        /**
         * Stores the position of the operator.
         */
        lexer::position operator_position;

        /**
         * Stores the query operator.
         */
        query_operator operator_ = static_cast<query_operator>(0);

        /**
         * Stores the attribute value.
         */
        basic_expression value;

        /**
         * Gets the context of the attribute query.
         * @return Returns the context of the attribute query.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for attribute query.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, attribute_query const& node);

    // Forward declaration of recursive query expressions
    struct nested_query_expression;

    /**
     * Represents a basic query expression.
     */
    struct basic_query_expression : boost::spirit::x3::variant<
        attribute_query,
        boost::spirit::x3::forward_ast<nested_query_expression>
        >
    {
        // Use the base's construction and assignment semantics
        using base_type::base_type;
        using base_type::operator=;

        /**
         * Default constructor for basic query expression.
         */
        basic_query_expression() = default;

        /**
         * Default copy constructor for basic query expression.
         */
        basic_query_expression(basic_query_expression const&) = default;

        /**
         * Default move constructor for basic query expression.
         */
        basic_query_expression(basic_query_expression&&) = default;

        /**
         * Default copy assignment operator for basic query expression.
         * @return Returns this basic query expression.
         */
        basic_query_expression& operator=(basic_query_expression const&) = default;

        /**
         * Default move assignment operator for basic query expression.
         * @return Returns this basic query expression.
         */
        basic_query_expression& operator=(basic_query_expression&&) = default;

        /**
         * Gets the context of the basic query expression.
         * @return Returns the context of the basic query expression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for basic query expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, basic_query_expression const& node);

    /**
     * Represents the possible binary query operators.
     */
    enum class binary_query_operator
    {
        /**
         * Logical "and" of queries.
         */
        logical_and,
        /**
         * Logical "or" of queries.
         */
        logical_or
    };

    /**
     * Stream insertion operator for binary query operator.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, binary_query_operator const& node);

    /**
     * Represents a binary query operation.
     */
    struct binary_query_operation
    {
        /**
         * Stores the position of the operator.
         */
        lexer::position operator_position;

        /**
         * Stores the binary query operator.
         */
        binary_query_operator operator_ = static_cast<binary_query_operator>(0);

        /**
         * Stores the right-hand side operand.
         */
        basic_query_expression operand;

        /**
         * Gets the context of the binary query operation.
         * @return Returns the context of the binary query operation.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for binary query operation.
     * @param os The output stream to write to.
     * @param operation The operation to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, binary_query_operation const& operation);

    /**
     * Represents a query expression.
     */
    struct query_expression
    {
        /**
         * Stores the first operand in the expression.
         */
        basic_query_expression operand;

        /**
         * Stores the binary operations of the query expression.
         */
        std::vector<binary_query_operation> operations;

        /**
         * Gets the context of the query expression.
         * @return Returns the context of the query expression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for query expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, query_expression const& node);

    /**
     * Represents a nested query expression.
     */
    struct nested_query_expression : context
    {
        /**
         * Stores the nested query expression.
         */
        query_expression expression;
    };

    /**
     * Stream insertion operator for nested query expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, nested_query_expression const& node);

    /**
     * Represents a collector expression.
     */
    struct collector_expression
    {
        /**
         * Stores the collector type.
         */
        ast::type type;

        /**
         * Stores whether or not exported resources are collected.
         */
        bool exported = false;

        /**
         * Stores the optional query expression.
         */
        boost::optional<query_expression> query;

        /**
         * The ending position of the expression.
         */
        lexer::position end;

        /**
         * Gets the context of the query expression.
         * @return Returns the context of the query expression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for collector expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, collector_expression const& node);

    /**
     * Represents a resource override reference.
     */
    struct resource_override_reference : boost::spirit::x3::variant<
        postfix_expression,
        collector_expression
        >
    {
        // Use the base's construction and assignment semantics
        using base_type::base_type;
        using base_type::operator=;

        /**
         * Default constructor for resource override reference.
         */
        resource_override_reference() = default;

        /**
         * Default copy constructor for resource override reference.
         */
        resource_override_reference(resource_override_reference const&) = default;

        /**
         * Default move constructor for resource override reference.
         */
        resource_override_reference(resource_override_reference&&) = default;

        /**
         * Default copy assignment operator for resource override reference.
         * @return Returns this resource override reference.
         */
        resource_override_reference& operator=(resource_override_reference const&) = default;

        /**
         * Default move assignment operator for resource override reference.
         * @return Returns this resource override reference.
         */
        resource_override_reference& operator=(resource_override_reference&&) = default;

        /**
         * Gets the context of the resource override reference.
         * @return Returns the context of the resource override reference.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for resource override reference.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource_override_reference const& node);

    /**
     * Represents a resource override expression.
     */
    struct resource_override_expression : context
    {
        /**
         * Stores the resource override reference.
         */
        resource_override_reference reference;

        /**
         * Stores the attribute operations.
         */
        std::vector<attribute_operation> operations;
    };

    /**
     * Stream insertion operator for resource override expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource_override_expression const& node);

    /**
     * Represents a relationship expression.
     */
    struct relationship_expression : boost::spirit::x3::variant<
        resource_declaration_expression,
        resource_override_expression,
        resource_defaults_expression,
        collector_expression,
        expression
        >
    {
        // Use the base's construction and assignment semantics
        using base_type::base_type;
        using base_type::operator=;

        /**
         * Default constructor for relationship expression.
         */
        relationship_expression() = default;

        /**
         * Default copy constructor for relationship expression.
         */
        relationship_expression(relationship_expression const&) = default;

        /**
         * Default move constructor for relationship expression.
         */
        relationship_expression(relationship_expression&&) = default;

        /**
         * Default copy assignment operator for relationship expression.
         * @return Returns this relationship expression.
         */
        relationship_expression& operator=(relationship_expression const&) = default;

        /**
         * Default move assignment operator for relationship expression.
         * @return Returns this relationship expression.
         */
        relationship_expression& operator=(relationship_expression&&) = default;

        /**
         * Gets the context of the relationship expression.
         * @return Returns the context of the relationship expression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for relationship expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, relationship_expression const& node);

    /**
     * Represents a relationship operator.
     */
    enum class relationship_operator
    {
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
     * Stream insertion operator for relationship operator.
     * @param os The output stream to write to.
     * @param op The operator to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, relationship_operator op);

    /**
     * Hashes a relationship operator.
     * @param op The operator to hash.
     * @return Returns the hash value for the relationship operator.
     */
    size_t hash_value(relationship_operator op);

    /**
     * Represents a relationship operation.
     */
    struct relationship_operation
    {
        /**
         * Stores the position of the operator.
         */
        lexer::position operator_position;

        /**
         * Stores the relationship operator.
         */
        relationship_operator operator_ = static_cast<relationship_operator>(0);

        /**
         * Stores the operand expression.
         */
        relationship_expression operand;

        /**
         * Gets the context of the relationship operation.
         * @return Returns the context of the relationship operation.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for relationship operation.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, relationship_operation const& node);

    /**
     * Represents a relationship statement.
     * A relationship statement (unlike other statements) produces a value.
     * Note that general expressions are treated like relationship statements with no relationship operations.
     * It is done this way for performance reasons; we don't want to backtrack when we fail to find a relationship operator.
     */
    struct relationship_statement
    {
        /**
         * Stores the first operand in the statement.
         */
        relationship_expression operand;

        /**
         * Stores the optional relationship operations.
         */
        std::vector<relationship_operation> operations;

        /**
         * Gets the context of the statement.
         * @return Returns the context of the statement.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for relationship statement.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, relationship_statement const& node);

    /**
     * Represents a supported serialization format for the syntax tree.
     */
    enum class format
    {
        /**
         * YAML format.
         */
        yaml
    };

    /**
     * Represents a Puppet syntax tree.
     */
    struct syntax_tree : std::enable_shared_from_this<syntax_tree>
    {
        /**
         * Gets the optional EPP parameters if parsed using the EPP rules.
         * @return Returns the optional EPP parameters.
         */
        boost::optional<std::vector<parameter>> parameters;

        /**
         * Stores the statements that make up the syntax tree.
         */
        std::vector<statement> statements;

        /**
         * Gets the path to the file represented by the syntax tree.
         * @return Returns the path to the file represented by the syntax tree.
         */
        std::string const& path() const;

        /**
         * Gets the path to the file represented by the syntax tree as a shared pointer.
         * @return Returns the pointer to the path.
         */
        std::shared_ptr<std::string> const& shared_path() const;

        /**
         * Gets the source code represented by the syntax tree.
         * @return Returns the source code represented by the syntax tree.
         */
        std::string const& source() const;

        /**
         * Sets the source code represented by the syntax tree.
         * @param source The source code represented by the syntax tree.
         */
        void source(std::string source);

        /**
         * Gets the module that owns this AST.
         * @return Returns the module that owns this AST.
         */
        compiler::module const* module() const;

        /**
         * Writes the syntax tree to a given stream.
         * @param format The format to serialize the syntax tree as.
         * @param stream The stream to write the syntax tree to.
         * @param include_path Specifies whether or not the serialized data should include the parsed file's path.
         */
        void write(ast::format format, std::ostream& stream, bool include_path = true) const;

        /**
         * Validates the AST.
         * Throws parse exceptions if validation fails.
         * @param epp True if the AST is from an EPP parse or false if not.
         */
        void validate(bool epp = false) const;

        /**
         * Creates a syntax tree.
         * @param path The path to the file represented by the syntax tree.
         * @param module The module that owns the AST.
         * @return Returns a shared pointer to the syntax tree.
         */
        static std::shared_ptr<syntax_tree> create(std::string path, compiler::module const* module = nullptr);

     protected:
        /**
         * Constructs a syntax tree.
         * @param path The path to the file that was parsed.
         * @param module The module that owns the AST.
         */
        syntax_tree(std::string path, compiler::module const* module);

     private:
        std::shared_ptr<std::string> _path;
        std::string _source;
        compiler::module const* _module;
    };

    /**
     * Stream insertion operator for syntax tree.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, syntax_tree const& node);

}}}  // namespace puppet::compiler::ast
