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
    begin,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::defaulted,
    begin,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::boolean,
    begin,
    value,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::regex,
    begin,
    value,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::variable,
    begin,
    name,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::name,
    begin,
    value,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::bare_word,
    begin,
    value,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::type,
    begin,
    name,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::postfix_expression,
    operand,
    operations
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::binary_operation,
    operator_position,
    operator_,
    operand
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::expression,
    operand,
    operations
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::interpolated_string,
    begin,
    format,
    parts,
    margin,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::array,
    begin,
    elements,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::hash,
    begin,
    elements,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::proposition,
    options,
    body,
    end
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::case_expression,
    begin,
    conditional,
    propositions,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::else_,
    begin,
    body,
    end
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::elsif,
    begin,
    conditional,
    body,
    end
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::if_expression,
    begin,
    conditional,
    body,
    end,
    elsifs,
    else_
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::unless_expression,
    begin,
    conditional,
    body,
    end,
    else_
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
    begin,
    parameters,
    body,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::function_call_expression,
    function,
    arguments,
    end,
    lambda
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::new_expression,
    type,
    arguments,
    end,
    lambda
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::epp_render_expression,
    begin,
    expression,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::epp_render_block,
    begin,
    block,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::epp_render_string,
    begin,
    string,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::unary_expression,
    operator_position,
    operator_,
    operand
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::nested_expression,
    begin,
    expression,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::selector_expression,
    begin,
    cases,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::access_expression,
    begin,
    arguments,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::method_call_expression,
    begin,
    method,
    arguments,
    end,
    lambda
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::attribute_operation,
    name,
    operator_position,
    operator_,
    value
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::resource_body,
    title,
    operations
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::resource_declaration_expression,
    begin,
    status,
    type,
    bodies,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::resource_defaults_expression,
    begin,
    type,
    operations,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::attribute_query,
    attribute,
    operator_position,
    operator_,
    value
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::binary_query_operation,
    operator_position,
    operator_,
    operand
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::query_expression,
    operand,
    operations
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::nested_query_expression,
    begin,
    expression,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::collector_expression,
    type,
    exported,
    query,
    end
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::resource_override_expression,
    begin,
    reference,
    operations,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::relationship_operation,
    operator_position,
    operator_,
    operand
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::class_statement,
    begin,
    name,
    parameters,
    parent,
    body,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::defined_type_statement,
    begin,
    name,
    parameters,
    body,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::node_statement,
    begin,
    hostnames,
    body,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::function_statement,
    is_private,
    begin,
    name,
    parameters,
    body,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::produces_statement,
    resource,
    capability,
    operations,
    end
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::consumes_statement,
    resource,
    capability,
    operations,
    end
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::application_statement,
    begin,
    name,
    parameters,
    body,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::site_statement,
    begin,
    body,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::type_alias_statement,
    begin,
    alias,
    type
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::function_call_statement,
    function,
    arguments,
    lambda
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::relationship_statement,
    operand,
    operations
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::break_statement,
    begin,
    end,
    tree
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::next_statement,
    begin,
    end,
    tree,
    value
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::return_statement,
    begin,
    end,
    tree,
    value
)

BOOST_FUSION_ADAPT_STRUCT(
    puppet::compiler::ast::syntax_tree,
    parameters,
    statements
)
