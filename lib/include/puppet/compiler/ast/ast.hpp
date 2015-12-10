/**
 * @file
 * Declares the AST structures.
 */
#pragma once

#include "../lexer/position.hpp"
#include "../lexer/number_token.hpp"
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

    // Forward declarations of recursive expressions
    // These expressions hold expressions themselves
    struct nested_expression;
    struct array;
    struct hash;
    struct case_expression;
    struct if_expression;
    struct unless_expression;
    struct access_expression;
    struct function_call_expression;
    struct resource_expression;
    struct resource_defaults_expression;
    struct resource_override_expression;
    struct class_expression;
    struct defined_type_expression;
    struct node_expression;
    struct query_expression;
    struct collector_expression;
    struct unary_expression;
    struct selector_expression;
    struct method_call_expression;
    struct postfix_expression;
    struct epp_render_expression;
    struct epp_render_block;
    struct epp_render_string;
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
     * Stream insertion operator for boolean.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, boolean const& node);

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
     * Represents a literal string.
     */
    struct string : context
    {
        /**
         * Stores the range of the value.
         */
        lexer::range value_range;

        /**
         * Stores the value of the literal string.
         */
        std::string value;

        /**
         * Stores the valid escape characters for the string.
         */
        std::string escapes;

        /**
         * Stores the data format of the string (heredocs only).
         */
        std::string format;

        /**
         * Stores the margin of the string (heredoc only).
         */
        int margin = 0;

        /**
         * Stores the opening quote character for the string.
         * For heredoc strings, this will be a null character.
         */
        char quote = '\'';

        /**
         * Stores whether or not the string is interpolated.
         */
        bool interpolated = false;

        /**
         * Stores whether or not a trailing break should be removed (heredoc only).
         */
        bool remove_break = false;
    };

    /**
     * Stream insertion operator for string.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, string const& node);

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
     * Represents a Puppet primary expression.
     */
    struct primary_expression : boost::spirit::x3::variant<
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
        boost::spirit::x3::forward_ast<nested_expression>,
        boost::spirit::x3::forward_ast<array>,
        boost::spirit::x3::forward_ast<hash>,
        boost::spirit::x3::forward_ast<case_expression>,
        boost::spirit::x3::forward_ast<if_expression>,
        boost::spirit::x3::forward_ast<unless_expression>,
        boost::spirit::x3::forward_ast<function_call_expression>,
        boost::spirit::x3::forward_ast<resource_expression>,
        boost::spirit::x3::forward_ast<resource_override_expression>,
        boost::spirit::x3::forward_ast<resource_defaults_expression>,
        boost::spirit::x3::forward_ast<class_expression>,
        boost::spirit::x3::forward_ast<defined_type_expression>,
        boost::spirit::x3::forward_ast<node_expression>,
        boost::spirit::x3::forward_ast<collector_expression>,
        boost::spirit::x3::forward_ast<unary_expression>,
        boost::spirit::x3::forward_ast<epp_render_expression>,
        boost::spirit::x3::forward_ast<epp_render_block>,
        boost::spirit::x3::forward_ast<epp_render_string>
        >
    {
        // Use the base's construction and assignment semantics
        using base_type::base_type;
        using base_type::operator=;

        /**
         * Default constructor for primary expression.
         */
        primary_expression() = default;

        /**
         * Default copy constructor for primary expression.
         */
        primary_expression(primary_expression const&) = default;

        /**
         * Default move constructor for primary expression.
         */
        primary_expression(primary_expression&&) = default;

        /**
         * Default copy assignment operator for primary expression.
         * @return Returns this primary expression.
         */
        primary_expression& operator=(primary_expression const&) = default;

        /**
         * Default move assignment operator for primary expression.
         * @return Returns this primary expression.
         */
        primary_expression& operator=(primary_expression&&) = default;

        /**
         * Get the context of the primary expression.
         * @return Returns the context of the primary expression.
         */
        ast::context context() const;

        /**
         * Determines if the expression is productive (i.e. has side effect).
         * @return Returns true if the expression is productive or false if not.
         */
        bool is_productive() const;

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
     * Stream insertion operator for primary expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, primary_expression const& node);

    /**
     * Represents a postfix subexpression.
     */
    struct postfix_subexpression : boost::spirit::x3::variant<
        boost::spirit::x3::forward_ast<selector_expression>,
        boost::spirit::x3::forward_ast<access_expression>,
        boost::spirit::x3::forward_ast<method_call_expression>
        >
    {
        // Use the base's construction and assignment semantics
        using base_type::base_type;
        using base_type::operator=;

        /**
         * Default constructor for postfix subexpression.
         */
        postfix_subexpression() = default;

        /**
         * Default copy constructor for postfix subexpression.
         */
        postfix_subexpression(postfix_subexpression const&) = default;

        /**
         * Default move constructor for postfix subexpression.
         */
        postfix_subexpression(postfix_subexpression&&) = default;

        /**
         * Default copy assignment operator for postfix subexpression.
         * @return Returns this postfix subexpression.
         */
        postfix_subexpression& operator=(postfix_subexpression const&) = default;

        /**
         * Default move assignment operator for postfix subexpression.
         * @return Returns this postfix subexpression.
         */
        postfix_subexpression& operator=(postfix_subexpression&&) = default;

        /**
         * Get the context of the postfix subexpression.
         * @return Returns the context of the postfix subexpression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for postfix subexpression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, postfix_subexpression const& node);

    /**
     * Represents a postfix expression.
     */
    struct postfix_expression
    {
        /**
         * Stores the primary expression.
         */
        primary_expression primary;

        /**
         * Stores the postfix subexpressions.
         */
        std::vector<postfix_subexpression> subexpressions;

        /**
         * Get the context of the postfix expression.
         * @return Returns the context of the postfix expression.
         */
        ast::context context() const;

        /**
         * Determines if the expression is productive (i.e. has side effect).
         * @return Returns true if the expression is productive or false if not.
         */
        bool is_productive() const;

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
     * Determines if the given binary operator is productive.
     * @param op The operator to check for being productive.
     * @return Returns true if the operator is productive or false if not.
     */
    bool is_productive(binary_operator op);

    /**
     * Hashes a binary operator.
     * @param op The operator to hash.
     * @return Returns the hash value for the binary operator.
     */
    size_t hash_value(binary_operator op);

    /**
     * Represents a binary operation.
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
         * Get the context of the binary operation.
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
         * Stores the postfix expression.
         */
        postfix_expression postfix;

        /**
         * Stores the binary operations of the expression.
         */
        std::vector<binary_operation> operations;

        /**
         * Get the context of the expression.
         * @return Returns the context of the expression.
         */
        ast::context context() const;

        /**
         * Determines if the expression is productive (i.e. has side effect).
         * @return Returns true if the expression is productive or false if not.
         */
        bool is_productive() const;

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
        std::vector<expression> body;

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
        std::vector<expression> body;

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
        std::vector<expression> body;

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
        std::vector<expression> body;

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
         * Get the context of the if expression.
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
        std::vector<expression> body;

        /**
         * Stores the ending position.
         */
        lexer::position end;

        /**
         * Stores the optional else.
         */
        boost::optional<ast::else_> else_;

        /**
         * Get the context of the unless expression.
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
         * Get the context of the parameter.
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
        std::vector<expression> body;
    };

    /**
     * Stream insertion operator for lambda expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, lambda_expression const& node);

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
         * Get the context of the method call expression.
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
         * Stores the optional lambda.
         */
        boost::optional<lambda_expression> lambda;

        /**
         * Stores the optional ending position.
         */
        boost::optional<lexer::position> end;

        /**
         * Get the context of the function call expression.
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
         * Get the context of the attribute operation.
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
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource_status const& node);

    /**
     * Represents a resource body.
     */
    struct resource_body
    {
        /**
         * Stores the resource title.
         */
        primary_expression title;

        /**
         * Stores the resource attribute operations.
         */
        std::vector<attribute_operation> operations;

        /**
         * Get the context of the resource body.
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
     * Represents a resource expression.
     */
    struct resource_expression : context
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
     * Stream insertion operator for resource expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, resource_expression const& node);

    /**
     * Represents a resource override expression.
     */
    struct resource_override_expression : context
    {
        /**
         * Stores the resource reference.
         */
        postfix_expression reference;

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
     * Represents a class expression.
     */
    struct class_expression : context
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
        std::vector<expression> body;
    };

    /**
     * Stream insertion operator for class expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, class_expression const& node);

    /**
     * Represents a defined type expression.
     */
    struct defined_type_expression : context
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
        std::vector<expression> body;
    };

    /**
     * Stream insertion operator for defined type expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, defined_type_expression const& node);

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
         * Get the context of the hostname.
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
     * Represents a node expression.
     */
    struct node_expression : context
    {
        /**
         * Stores the hostnames.
         */
        std::vector<hostname> hostnames;

        /**
         * Stores the body.
         */
        std::vector<expression> body;
    };

    /**
     * Stream insertion operator for node expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, node_expression const& node);

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
        primary_expression value;

        /**
         * Get the context of the attribute query.
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

    /**
     * Represents a primary query expression.
     */
    struct primary_query_expression : boost::spirit::x3::variant<
        attribute_query,
        boost::spirit::x3::forward_ast<query_expression>
        >
    {
        // Use the base's construction and assignment semantics
        using base_type::base_type;
        using base_type::operator=;

        /**
         * Default constructor for primary query expression.
         */
        primary_query_expression() = default;

        /**
         * Default copy constructor for primary query expression.
         */
        primary_query_expression(primary_query_expression const&) = default;

        /**
         * Default move constructor for primary query expression.
         */
        primary_query_expression(primary_query_expression&&) = default;

        /**
         * Default copy assignment operator for primary query expression.
         * @return Returns this primary query expression.
         */
        primary_query_expression& operator=(primary_query_expression const&) = default;

        /**
         * Default move assignment operator for primary query expression.
         * @return Returns this primary query expression.
         */
        primary_query_expression& operator=(primary_query_expression&&) = default;

        /**
         * Get the context of the primary query expression.
         * @return Returns the context of the primary query expression.
         */
        ast::context context() const;
    };

    /**
     * Stream insertion operator for primary query expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, primary_query_expression const& node);

    /**
     * Represents the possible binary query operators.
     */
    enum class binary_query_operator
    {
        /**
         * No operator.
         */
        none,

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
        primary_query_expression operand;

        /**
         * Get the context of the binary query operation.
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
         * Stores the primary query expression.
         */
        primary_query_expression primary;

        /**
         * Stores the binary operations of the query expression.
         */
        std::vector<binary_query_operation> operations;

        /**
         * Get the context of the query expression.
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
         * Get the context of the query expression.
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
     * Stream insertion operator for unary operator.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, unary_operator const& node);

    /**
     * Hashes a unary operator.
     * @param operator_ The operator to hash.
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
          * Get the context of the unary expression.
          * @return Returns the context of the unary expression.
          */
        ast::context context() const;

        /**
         * Determines if the expression is a splat.
         * @return Returns true if the expression is a splat or false if not.
         */
        bool is_splat() const;
    };

    /**
     * Stream insertion operator for unary expression.
     * @param os The output stream to write to.
     * @param node The node to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, unary_expression const& node);

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
        std::vector<expression> statements;

        /**
         * Stores the ending position for string interpolation.
         * This member will be default constructed if not parsed for string interpolation.
         */
        lexer::position end;

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
        explicit syntax_tree(std::string path, compiler::module const* module);

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
