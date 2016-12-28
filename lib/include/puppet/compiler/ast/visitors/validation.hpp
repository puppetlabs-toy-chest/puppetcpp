/**
 * @file
 * Declares the validation visitor.
 */
#pragma once

#include "../ast.hpp"
#include <boost/variant.hpp>
#include <functional>
#include <deque>

namespace puppet { namespace compiler { namespace ast { namespace visitors {

    /**
     * A visitor for AST validation.
     */
    struct validation : boost::static_visitor<void>
    {
        /**
         * Constructs a validation visitor.
         * @param epp True if validating EPP syntax trees or false for not.
         * @param allow_catalog_statements True if catalog statements are allowed or false if not.
         */
        explicit validation(bool epp = false, bool allow_catalog_statements = true);

        /**
         * Visits the given AST.
         * @param tree The tree to visit.
         */
        void visit(syntax_tree const& tree);

        /**
         * Visits the given statement.
         * @param statement The statement to validate.
         * @param effective True if the statement is required to be effective or false if not.
         */
        void visit(ast::statement const& statement, bool effective = false);

     private:
        enum class location
        {
            top,
            epp,
            case_,
            if_,
            unless,
            lambda,
            class_,
            defined_type,
            node,
            function,
            application,
            site
        };

        template<class> friend class ::boost::detail::variant::invoke_visitor;
        void operator()(basic_expression const& expression);
        void operator()(undef const&);
        void operator()(defaulted const&);
        void operator()(boolean const& expression);
        void operator()(number const& expression);
        void operator()(ast::string const& expression);
        void operator()(ast::regex const& expression);
        void operator()(variable const& expression);
        void operator()(name const& expression);
        void operator()(bare_word const& expression);
        void operator()(type const& expression);
        void operator()(interpolated_string const& expression);
        void operator()(literal_string_text const&);
        void operator()(ast::array const& expression);
        void operator()(hash const& expression);
        void operator()(case_expression const& expression);
        void operator()(if_expression const& expression);
        void operator()(unless_expression const& expression);
        void operator()(function_call_expression const& expression);
        void operator()(lambda_expression const& expression);
        void operator()(new_expression const& expression);
        void operator()(epp_render_expression const& expression);
        void operator()(epp_render_block const& expression);
        void operator()(epp_render_string const& expression);
        void operator()(unary_expression const& expression);
        void operator()(nested_expression const& expression);
        void operator()(ast::expression const& expression);
        void operator()(postfix_expression const& expression);
        void operator()(selector_expression const& expression);
        void operator()(access_expression const& expression);
        void operator()(method_call_expression const& expression);
        void operator()(ast::statement const& statement, bool effective);
        void operator()(class_statement const& statement);
        void operator()(defined_type_statement const& statement);
        void operator()(node_statement const& statement);
        void operator()(function_statement const& statement);
        void operator()(produces_statement const& statement);
        void operator()(consumes_statement const& statement);
        void operator()(application_statement const& statement);
        void operator()(site_statement const& statement);
        void operator()(type_alias_statement const& statement);
        void operator()(function_call_statement const& statement);
        void operator()(relationship_statement const& statement);
        void operator()(relationship_expression const& expression);
        void operator()(resource_declaration_expression const& expression);
        void operator()(resource_override_expression const& expression);
        void operator()(resource_defaults_expression const& expression);
        void operator()(collector_expression const& expression);
        void operator()(query_expression const& expression);
        void operator()(nested_query_expression const& expression);
        void operator()(basic_query_expression const& expression);
        void operator()(attribute_query const& expression);

        void validate_parameters(std::vector<parameter> const& parameters, bool is_resource = false, bool pass_by_hash = false);
        void validate_parameter_name(parameter const& parameter, bool is_resource_parameter) const;
        location current_location() const;
        void validate_body(std::vector<statement> const& body, bool has_return_value);
        void validate_assignment_operand(postfix_expression const& operand);
        void validate_assignment_operand(ast::array const& operand);
        void validate_assignment_operand(variable const& operand);
        void validate_catalog_statement(ast::context const& context);

        struct location_helper
        {
            location_helper(validation& visitor, location where);
            ~location_helper();

         private:
            validation& _visitor;
        };

        struct parameter_helper
        {
            parameter_helper(validation& visitor, parameter const* begin, parameter const* end);
            ~parameter_helper();

         private:
            validation& _visitor;
        };

        std::deque<location> _locations;
        parameter const* _parameter_begin = nullptr;
        parameter const* _parameter_end = nullptr;
        bool _epp = false;
        bool _allow_catalog_statements = true;
    };

}}}}  // namespace puppet::compiler::visitors
