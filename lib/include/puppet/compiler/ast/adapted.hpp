/**
 * @file
 * Declares Boost.Fusion adaptations of AST structures.
 */
#pragma once

#include "ast.hpp"
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::undef,
    context
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::defaulted,
    context
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::boolean,
    context,
    value
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::regex,
    context,
    value
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::variable,
    context,
    name
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::name,
    context,
    value
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::bare_word,
    context,
    value
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::type,
    context,
    name
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::array,
    context,
    elements
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::hash,
    context,
    elements
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::selector_expression,
    context,
    cases
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::case_proposition,
    options,
    body
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::case_expression,
    context,
    conditional,
    propositions
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::else_expression,
    context,
    body
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::elsif_expression,
    context,
    conditional,
    body
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::if_expression,
    context,
    conditional,
    body,
    elsifs,
    else_
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::unless_expression,
    context,
    conditional,
    body,
    else_
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::access_expression,
    context,
    arguments
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::parameter,
    type,
    captures,
    variable,
    default_value
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::lambda_expression,
    context,
    parameters,
    body
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::method_call_expression,
    context,
    method,
    arguments,
    lambda
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::function_call_expression,
    function,
    arguments,
    lambda
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::attribute,
    name,
    oper,
    value
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::resource_body,
    title,
    attributes
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::resource_expression,
    status,
    type,
    bodies
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::resource_override_expression,
    reference,
    attributes
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::resource_defaults_expression,
    type,
    attributes
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::class_expression,
    context,
    name,
    parameters,
    parent,
    body
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::defined_type_expression,
    context,
    name,
    parameters,
    body
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::node_expression,
    context,
    hostnames,
    body
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::attribute_query,
    attribute,
    oper,
    value
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::binary_attribute_query,
    context,
    oper,
    operand
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::collector_query_expression,
    primary,
    remainder
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::collector_expression,
    type,
    exported,
    query
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::postfix_expression,
    primary,
    subexpressions
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::unary_expression,
    context,
    oper,
    operand
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::binary_expression,
    context,
    oper,
    operand
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::expression,
    postfix,
    remainder
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::syntax_tree,
    statements,
    closing_position
)
