/**
 * @file
 * Declares the AST collection expression.
 */
#pragma once

#include "expression.hpp"
#include <vector>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Represents the possible attribute query operators.
     */
    enum class attribute_query_operator
    {
        /**
         * No operator.
         */
        none,

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
     * Stream insertion operator for AST attribute query operator.
     * @param os The output stream to write the operator to.
     * @param op The operator to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, attribute_query_operator const& op);

    /**
     * Represents a collection query.
     */
    struct query
    {
        /**
         * Default constructor for query.
         */
        query();

        /**
         * Constructs a query with the given attribute, operator, and value.
         * @param attribute The attribute being queried.
         * @param op The attribute query operator.
         * @param value The query value.
         */
        query(name attribute, attribute_query_operator op, basic_expression value);

        /**
         * Gets the attribute being queried.
         * @return Returns the attribute being queried.
         */
        name const& attribute() const;

        /**
         * Gets the attribute being queried.
         * @return Returns the attribute being queried.
         */
        name& attribute();

        /**
         * Gets the attribute query operator.
         * @return Returns the query operator.
         */
        attribute_query_operator op() const;

        /**
         * Gets the attribute query operator.
         * @return Returns the query operator.
         */
        attribute_query_operator& op();

        /**
         * Gets the query value.
         * @return Returns the query value.
         */
        basic_expression const& value() const;

        /**
         * Gets the query value.
         * @return Returns the query value.
         */
        basic_expression& value();

        /**
         * Gets the position of the query.
         * @return Returns the position of the query.
         */
        lexer::token_position const& position() const;

     private:
        name _attribute;
        attribute_query_operator _op;
        basic_expression _value;
    };

    /**
     * Stream insertion operator for AST collection query.
     * @param os The output stream to write the query to.
     * @param query The query to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::query const& query);

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
     * Stream insertion operator for AST collection query operator.
     * @param os The output stream to write the operator to.
     * @param op The operator to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, binary_query_operator const& op);

    /**
     * Represents an AST collection binary query expression.
     */
    struct binary_query_expression
    {
        /**
         * Default constructor for binary_query_expression.
         */
        binary_query_expression();

        /**
         * Constructs a binary query expression with the given query operator and right-hand side.
         * @param op The binary query operator used in the expression.
         * @param operand The right-hand query operand.
         */
        binary_query_expression(binary_query_operator op, query operand);

        /**
         * Gets the binary query operator in the expression.
         * @return Returns the binary query operator in the expression.
         */
        binary_query_operator op() const;

        /**
         * Gets the binary query operator in the expression.
         * @return Returns the binary query operator in the expression.
         */
        binary_query_operator& op();

        /**
         * Gets the right-hand operand of the binary query expression.
         * @return Returns the right-hand operand of the binary query expression.
         */
        query const& operand() const;

        /**
         * Gets the right-hand operand of the binary query expression.
         * @return Returns the right-hand operand of the binary query expression.
         */
        query& operand();

        /**
         * Gets the position of the binary query expression.
         * @return Returns the position of the binary query expression.
         */
        lexer::token_position const& position() const;

     private:
        binary_query_operator _op;
        query _operand;
    };

    /**
     * Stream insertion operator for AST binary query expression.
     * @param os The output stream to write the expression to.
     * @param expr The expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, binary_query_expression const& expr);

    /**
     * Represents the kind of collection expression.
     */
    enum class collection_kind
    {
        /**
         * No operator.
         */
        none,

        /**
         * Collects all resources.
         */
        all,

        /**
         * Collects only exported resources.
         */
        exported
    };

    /**
     * Represents an AST collection expression.
     */
    struct collection_expression
    {
        /**
         * Default constructor for collection_expression.
         */
        collection_expression();

        /**
         * Constructs a collection expression with the given type, first query, and binary query expressions.
         * @param kind The kind of collection expression.
         * @param type The type being collected.
         * @param first The optional first query.
         * @param remainder The remaining binary query expressions.
         */
        collection_expression(collection_kind kind, ast::type type, boost::optional<query> first, std::vector<binary_query_expression> remainder = std::vector<binary_query_expression>());

        /**
         * Gets the kind of collection expression.
         * @return Returns the kind of the collection expression.
         */
        collection_kind kind() const;

        /**
         * Gets the kind of collection expression.
         * @return Returns the kind of the collection expression.
         */
        collection_kind& kind();

        /**
         * Gets the type being collected.
         * @return Returns the type being collected.
         */
        ast::type const& type() const;

        /**
         * Gets the type being collected.
         * @return Returns the type being collected.
         */
        ast::type& type();

        /**
         * Gets the optional first query in the expression.
         * @return Returns the first query in the expression.
         */
        boost::optional<query> const& first() const;

        /**
         * Gets the optional first query in the expression.
         * @return Returns the first query in the expression.
         */
        boost::optional<query>& first();

        /**
         * Gets the remainder of the expression (for binary query expressions).
         * @return Returns the remainder of the expression.
         */
        std::vector<binary_query_expression> const& remainder() const;

        /**
         * Gets the remainder of the expression (for binary query expressions).
         * @return Returns the remainder of the expression.
         */
        std::vector<binary_query_expression>& remainder();

        /**
         * Gets the position of the expression.
         * @return Returns the position of the expression.
         */
        lexer::token_position const& position() const;

    private:
        collection_kind _kind;
        ast::type _type;
        boost::optional<query> _first;
        std::vector<binary_query_expression> _remainder;
    };

    /**
     * Stream insertion operator for AST collection expression.
     * @param os The output stream to write the expression to.
     * @param expr The expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, collection_expression const& expr);

}}  // namespace puppet::ast
