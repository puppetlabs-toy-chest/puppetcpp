/**
 * @file
 * Declares the type specification validation visitor.
 */
#pragma once

#include "../ast.hpp"
#include <boost/variant.hpp>
#include <functional>

namespace puppet { namespace compiler { namespace ast { namespace visitors {

    /**
     * A visitor for validating type specifications.
     */
    struct type : boost::static_visitor<void>
    {
        /**
         * Visits the given postfix expression.
         * @param expression The postfix expression to validate as a type specification.
         */
        void visit(postfix_expression const& expression);

     private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;
        void operator()(undef const&);
        void operator()(defaulted const&);
        void operator()(boolean const& expression);
        void operator()(number const& expression);
        void operator()(ast::string const& expression);
        void operator()(ast::regex const& expression);
        void operator()(variable const& expression);
        void operator()(name const& expression);
        void operator()(bare_word const& expression);
        void operator()(ast::type const& expression);
        void operator()(literal_string_text const& expression);
        void operator()(interpolated_string const& expression);
        void operator()(ast::array const& expression);
        void operator()(hash const& expression);
        void operator()(case_expression const& expression);
        void operator()(if_expression const& expression);
        void operator()(unless_expression const& expression);
        void operator()(function_call_expression const& expression);
        void operator()(new_expression const& expression);
        void operator()(epp_render_expression const& expression);
        void operator()(epp_render_block const& expression);
        void operator()(epp_render_string const& expression);
        void operator()(unary_expression const& expression);
        void operator()(nested_expression const& expression);
        void operator()(basic_expression const& expression);
        void operator()(ast::expression const& expression);
        void operator()(postfix_expression const& expression);
        void operator()(selector_expression const& expression);
        void operator()(access_expression const& expression);
        void operator()(method_call_expression const& expression);
    };

}}}}  // namespace puppet::compiler::visitors
