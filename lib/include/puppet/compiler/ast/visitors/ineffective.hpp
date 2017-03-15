/**
 * @file
 * Declares the ineffective visitor.
 */
#pragma once

#include "../ast.hpp"
#include <boost/variant.hpp>
#include <functional>
#include <deque>

namespace puppet { namespace compiler { namespace ast { namespace visitors {

    /**
     * A visitor for determining if a statement is ineffective.
     */
    struct ineffective : boost::static_visitor<bool>
    {
        /**
         * Visits the given statement.
         * @param statement The statement to check.
         * @return Returns true if the statement is ineffective or false if it is effective.
         */
        bool visit(ast::statement const& statement) const;

     private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;
        bool operator()(basic_expression const& expression) const;
        bool operator()(undef const&) const;
        bool operator()(defaulted const&) const;
        bool operator()(boolean const& expression) const;
        bool operator()(number const& expression) const;
        bool operator()(ast::string const& expression) const;
        bool operator()(ast::regex const& expression) const;
        bool operator()(variable const& expression) const;
        bool operator()(name const& expression) const;
        bool operator()(bare_word const& expression) const;
        bool operator()(type const& expression) const;
        bool operator()(interpolated_string const& expression) const;
        bool operator()(literal_string_text const&) const;
        bool operator()(ast::array const& expression) const;
        bool operator()(hash const& expression) const;
        bool operator()(case_expression const& expression) const;
        bool operator()(if_expression const& expression) const;
        bool operator()(unless_expression const& expression) const;
        bool operator()(function_call_expression const& expression) const;
        bool operator()(lambda_expression const& expression) const;
        bool operator()(new_expression const& expression) const;
        bool operator()(epp_render_expression const& expression) const;
        bool operator()(epp_render_block const& expression) const;
        bool operator()(epp_render_string const& expression) const;
        bool operator()(unary_expression const& expression) const;
        bool operator()(nested_expression const& expression) const;
        bool operator()(postfix_expression const& expression) const;
        bool operator()(postfix_operation const& operation) const;
        bool operator()(selector_expression const& expression) const;
        bool operator()(access_expression const& expression) const;
        bool operator()(method_call_expression const& expression) const;
        bool operator()(ast::expression const& expression) const;
        bool operator()(ast::statement const& statement) const;
        bool operator()(class_statement const& statement) const;
        bool operator()(defined_type_statement const& statement) const;
        bool operator()(node_statement const& statement) const;
        bool operator()(function_statement const& statement) const;
        bool operator()(produces_statement const& statement) const;
        bool operator()(consumes_statement const& statement) const;
        bool operator()(application_statement const& statement) const;
        bool operator()(site_statement const& statement) const;
        bool operator()(type_alias_statement const& statement) const;
        bool operator()(function_call_statement const& statement) const;
        bool operator()(relationship_statement const& statement) const;
        bool operator()(relationship_expression const& expression) const;
        bool operator()(resource_declaration_expression const& expression) const;
        bool operator()(resource_override_expression const& expression) const;
        bool operator()(resource_defaults_expression const& expression) const;
        bool operator()(collector_expression const& expression) const;
        bool operator()(break_statement const& statement) const;
    };

}}}}  // namespace puppet::compiler::visitors
