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
         * The attribute name to query.
         */
        name attribute;

        /**
         * The attribute query operator.
         */
        attribute_query_operator op;

        /**
         * The attribute value to query.
         */
        basic_expression value;
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
         * The binary query operator.
         */
        binary_query_operator op;

        /**
         * The right-hand operand.
         */
        query operand;
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
         * The collection kind.
         */
        collection_kind kind;

        /**
         * The type being collected.
         */
        ast::type type;

        /**
         * The first query.
         */
        boost::optional<query> first;

        /**
         * The remaining binary query expressions.
         */
        std::vector<binary_query_expression> remainder;

        /**
         * Gets the position of the expression.
         */
        lexer::position const& position() const;
    };

    /**
     * Stream insertion operator for AST collection expression.
     * @param os The output stream to write the expression to.
     * @param expr The expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, collection_expression const& expr);

}}  // namespace puppet::ast
