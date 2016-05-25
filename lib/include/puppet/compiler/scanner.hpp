/**
 * @file
 * Declares the definition scanner.
 */
#pragma once

#include "ast/ast.hpp"
#include "registry.hpp"
#include "evaluation/dispatcher.hpp"
#include <boost/variant.hpp>
#include <vector>
#include <string>
#include <memory>

namespace puppet { namespace compiler {

    /**
     * Represents the definition scanner.
     */
    struct scanner : boost::static_visitor<>
    {
        /**
         * Constructs a definition scanner.
         * @param registry The registry to populate with definitions.
         * @param dispatcher The dispatcher to populate with definitions.
         */
        scanner(compiler::registry& registry, evaluation::dispatcher& dispatcher);

        /**
         * Scans the given syntax tree for definitions.
         * Throws parse_exception if definition validation fails.
         * @param tree The syntax tree to scan for definitions.
         */
        void scan(ast::syntax_tree const& tree);

        /**
         * Scans the given expression for definitions.
         * Throws parse_exception if definition validation fails.
         * @param expression The expression to scan.
         * @return Returns true if the expression contained a definition or false if it did not.
         */
        bool scan(ast::expression const& expression);

     private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;
        void operator()(ast::undef const&);
        void operator()(ast::defaulted const&);
        void operator()(ast::boolean const& expression);
        void operator()(ast::number const& expression);
        void operator()(ast::string const& expression);
        void operator()(ast::regex const& expression);
        void operator()(ast::variable const& expression);
        void operator()(ast::name const& expression);
        void operator()(ast::bare_word const& expression);
        void operator()(ast::type const& expression);
        void operator()(ast::literal_string_text const& expression);
        void operator()(ast::interpolated_string const& expression);
        void operator()(ast::array const& expression);
        void operator()(ast::hash const& expression);
        void operator()(ast::expression const& expression);
        void operator()(ast::nested_expression const& expression);
        void operator()(ast::case_expression const& expression);
        void operator()(ast::if_expression const& expression);
        void operator()(ast::unless_expression const& expression);
        void operator()(ast::function_call_expression const& expression);
        void operator()(ast::new_expression const& expression);
        void operator()(ast::resource_expression const& expression);
        void operator()(ast::resource_override_expression const& expression);
        void operator()(ast::resource_defaults_expression const& expression);
        void operator()(ast::class_expression const& expression);
        void operator()(ast::defined_type_expression const& expression);
        void operator()(ast::node_expression const& expression);
        void operator()(ast::collector_expression const& expression);
        void operator()(ast::query_expression const& expression);
        void operator()(ast::nested_query_expression const& expression);
        void operator()(ast::primary_query_expression const& expression);
        void operator()(ast::attribute_query const& expression);
        void operator()(ast::function_expression const& expression);
        void operator()(ast::unary_expression const& expression);
        void operator()(ast::postfix_expression const& expression);
        void operator()(ast::selector_expression const& expression);
        void operator()(ast::access_expression const& expression);
        void operator()(ast::method_call_expression const& expression);
        void operator()(ast::lambda_expression const& expression);
        void operator()(ast::parameter const& expression);
        void operator()(ast::primary_expression const& expression);
        void operator()(ast::epp_render_expression const& expression);
        void operator()(ast::epp_render_block const& expression);
        void operator()(ast::epp_render_string const& expression);
        void operator()(ast::produces_expression const& expression);
        void operator()(ast::consumes_expression const& expression);
        void operator()(ast::application_expression const& expression);
        void operator()(ast::site_expression const& expression);
        void operator()(ast::type_alias_expression const& expression);
        bool can_define() const;
        std::string qualify(std::string const& name) const;
        std::string validate_name(bool is_class, ast::name const& name) const;
        void validate_parameters(char const* type, std::vector<ast::parameter> const& parameters) const;
        void validate_parameter_name(ast::parameter const& parameter) const;

        registry& _registry;
        evaluation::dispatcher& _dispatcher;
        std::vector<ast::class_expression const*> _scopes;
        bool _found_definition;
    };

}}  // puppet::compiler
