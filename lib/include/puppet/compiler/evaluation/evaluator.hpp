/**
 * @file
 * Declares the Puppet language expression evaluator.
 */
#pragma once

#include "context.hpp"
#include "../ast/ast.hpp"
#include "../../runtime/values/value.hpp"
#include <vector>
#include <memory>
#include <unordered_set>

namespace puppet { namespace compiler { namespace evaluation {

    /**
     * Represents the Puppet language expression evaluator.
     */
    struct evaluator : boost::static_visitor<runtime::values::value>
    {
        /**
         * Constructs an expression evaluator.
         * @param context The current evaluation context.
         */
        explicit evaluator(evaluation::context& context);

        /**
         * Gets the current evaluation context.
         * @return Returns the current evaluation context.
         */
        evaluation::context& context();

        /**
         * Evaluates all statements in a syntax tree.
         * @param tree The syntax tree to evaluate.
         * @param arguments The arguments for the tree (EPP syntax trees).
         * @return Returns the value produced by the last statement in the AST.
         */
        runtime::values::value evaluate(ast::syntax_tree const& tree, runtime::values::hash* arguments = nullptr);

        /**
         * Evaluates the given statement and returns the resulting runtime value.
         * @param statement The statement to evaluate.
         * @return Returns the runtime value that is the result of evaluating the statement.
         */
        runtime::values::value evaluate(ast::statement const& statement);

        /**
         * Evaluates the given statements and returns the value produced by the last statement.
         * @param statements The statements to evaluate.
         * @return Returns the value produced by the last statement or undef if there were no statements given.
         */
        runtime::values::value evaluate(std::vector<ast::statement> const& statements);

        /**
         * Evaluates the given expression and returns the resulting runtime value.
         * @param expression The expression to evaluate.
         * @return Returns the runtime value that is the result of evaluating the expression.
         */
        runtime::values::value evaluate(ast::expression const& expression);

        /**
         * Evaluates the given postfix expression and returns the resulting runtime value.
         * @param expression The expression to evaluate.
         * @return Returns the runtime value that is the result of evaluating the expression.
         */
        runtime::values::value evaluate(ast::postfix_expression const& expression);

        /**
         * Evaluates the given basic expression and returns the resulting runtime value.
         * @param expression The expression to evaluate.
         * @return Returns the runtime value that is the result of evaluating the expression.
         */
        runtime::values::value evaluate(ast::basic_expression const& expression);

     private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;
        runtime::values::value operator()(ast::basic_expression const& expression);
        runtime::values::value operator()(ast::undef const&);
        runtime::values::value operator()(ast::defaulted const&);
        runtime::values::value operator()(ast::boolean const& expression);
        runtime::values::value operator()(int64_t value);
        runtime::values::value operator()(double value);
        runtime::values::value operator()(ast::number const& expression);
        runtime::values::value operator()(ast::string const& expression);
        runtime::values::value operator()(ast::regex const& expression);
        runtime::values::value operator()(ast::variable const& expression);
        runtime::values::value operator()(ast::name const& expression);
        runtime::values::value operator()(ast::bare_word const& expression);
        runtime::values::value operator()(ast::type const& expression);
        runtime::values::value operator()(ast::interpolated_string const& expression);
        runtime::values::value operator()(ast::array const& expression);
        runtime::values::value operator()(ast::hash const& expression);
        runtime::values::value operator()(ast::case_expression const& expression);
        runtime::values::value operator()(ast::if_expression const& expression);
        runtime::values::value operator()(ast::unless_expression const& expression);
        runtime::values::value operator()(ast::function_call_expression const& expression);
        runtime::values::value operator()(ast::new_expression const& expression);
        runtime::values::value operator()(ast::epp_render_expression const& expression);
        runtime::values::value operator()(ast::epp_render_block const& expression);
        runtime::values::value operator()(ast::epp_render_string const& expression);
        runtime::values::value operator()(ast::unary_expression const& expression);
        runtime::values::value operator()(ast::nested_expression const& expression);
        runtime::values::value operator()(ast::expression const& expression);
        runtime::values::value operator()(ast::postfix_expression const& expression);
        runtime::values::value operator()(ast::statement const& statement);
        runtime::values::value operator()(ast::class_statement const& statement);
        runtime::values::value operator()(ast::defined_type_statement const& statement);
        runtime::values::value operator()(ast::node_statement const& statement);
        runtime::values::value operator()(ast::function_statement const& statement);
        runtime::values::value operator()(ast::produces_statement const& statement);
        runtime::values::value operator()(ast::consumes_statement const& statement);
        runtime::values::value operator()(ast::application_statement const& statement);
        runtime::values::value operator()(ast::site_statement const& statement);
        runtime::values::value operator()(ast::type_alias_statement const& statement);
        runtime::values::value operator()(ast::function_call_statement const& statement);
        runtime::values::value operator()(ast::relationship_statement const& statement);
        runtime::values::value operator()(ast::relationship_expression const& expression);
        runtime::values::value operator()(ast::resource_declaration_expression const& expression);
        runtime::values::value operator()(ast::resource_override_expression const& expression);
        runtime::values::value operator()(ast::resource_defaults_expression const& expression);
        runtime::values::value operator()(ast::collector_expression const& expression);
        runtime::values::value operator()(ast::break_statement const& statement);
        runtime::values::value operator()(ast::next_statement const& statement);

        ast::resource_body const* find_default_body(ast::resource_declaration_expression const& expression);
        attributes evaluate_attributes(bool is_class, std::vector<ast::attribute_operation> const& operations);
        void splat_attribute(compiler::attributes& attributes, std::unordered_set<std::string>& names, ast::attribute_operation const& operations);
        void validate_attribute(std::string const& name, runtime::values::value& value, ast::context const& context);
        std::vector<resource*> create_resources(bool is_class, std::string const& type_name, ast::resource_declaration_expression const& expression, attributes const& defaults);

        std::pair<runtime::values::value, ast::context> climb_expression(
            ast::postfix_expression const& expression,
            unsigned int min_precedence,
            std::vector<ast::binary_operation>::const_iterator& begin,
            std::vector<ast::binary_operation>::const_iterator const& end);

        void align_text(std::string const& text, size_t margin, size_t& current_margin, std::function<void(char const*, size_t)> const& callback);

        evaluation::context& _context;
    };

    /**
     * Represents a Puppet function evaluator.
     */
    struct function_evaluator
    {
        /**
         * Constructs a function evaluator for a Puppet function.
         * @param context The current evaluation context.
         * @param statement The Puppet function statement.
         */
        function_evaluator(evaluation::context& context, ast::function_statement const& statement);

        /**
         * Constructs a function evaluator for a named function without a definition.
         * @param context The current evaluation context.
         * @param name The function's name.
         * @param parameters The function's parameters.
         * @param body The function's body.
         */
        function_evaluator(evaluation::context& context, char const* name, std::vector<ast::parameter> const& parameters, std::vector<ast::statement> const& body);

        /**
         * Evaluates the function.
         * @param arguments The arguments passed to the function.
         * @param parent The parent scope to use or nullptr for top scope.
         * @param call_context The context of the call expression.
         * @param allow_excessive If true, excessive arguments are not an error.  If false, an error is raised when there are excessive arguments.
         * @return Returns the function's return value.
         */
        runtime::values::value evaluate(
            runtime::values::array& arguments,
            std::shared_ptr<scope> parent = nullptr,
            ast::context const& call_context = {},
            bool allow_excessive = true) const;

        /**
         * Evaluates the function.
         * @param arguments The arguments passed to the function.
         * @param parent The parent scope to use or nullptr for top scope.
         * @return Returns the function's return value.
         */
        runtime::values::value evaluate(runtime::values::hash& arguments, std::shared_ptr<scope> parent = nullptr) const;

     private:
        evaluation::context& _context;
        char const* _name;
        ast::function_statement const* _statement;
        std::vector<ast::parameter> const& _parameters;
        std::vector<ast::statement> const& _body;
    };

    /**
     * Represents a Puppet resource evaluator.
     * Base type for class, defined type, and node evaluators.
     */
    struct resource_evaluator
    {
        /**
         * Constructs a resource evaluator.
         * @param context The current evaluation context.
         * @param parameters The resource definition's parameters.
         * @param body The resource definition's body.
         */
        resource_evaluator(evaluation::context& context, std::vector<ast::parameter> const& parameters, std::vector<ast::statement> const& body);

     protected:
        /**
         * Prepares the scope.
         * @param scope The scope to prepare for evaluation.
         * @param resource The resource whose attributes will be set in the scope.
         */
        void prepare_scope(evaluation::scope& scope, compiler::resource& resource) const;

        /**
         * Stores the evaluation context.
         */
        evaluation::context& _context;

        /**
         * Stores the parameters.
         */
        std::vector<ast::parameter> const& _parameters;

        /**
         * Stores the body.
         */
        std::vector<ast::statement> const& _body;
    };

    /**
     * Represents a Puppet class evaluator.
     */
    struct class_evaluator : resource_evaluator
    {
        /**
         * Constructs a class evaluator.
         * @param context The current evaluation context.
         * @param statement The class statement to evaluate.
         */
        class_evaluator(evaluation::context& context, ast::class_statement const& statement);

        /**
         * Evaluates for the given resource.
         * @param resource The class resource to evaluate.
         */
        void evaluate(compiler::resource& resource) const;

     private:
        std::shared_ptr<scope> evaluate_parent() const;
        ast::class_statement const& _statement;
    };

    /**
     * Represents a Puppet defined type evaluator.
     */
    struct defined_type_evaluator : resource_evaluator
    {
        /**
         * Constructs a defined type evaluator.
         * @param context The current evaluation context.
         * @param statement The defined type statement to evaluate.
         */
        defined_type_evaluator(evaluation::context& context, ast::defined_type_statement const& statement);

        /**
         * Evaluates for the given resource.
         * @param resource The defined type resource to evaluate.
         */
        void evaluate(compiler::resource& resource) const;

     private:
        ast::defined_type_statement const& _statement;
    };

    /**
     * Represents a Puppet node evaluator.
     */
    struct node_evaluator : resource_evaluator
    {
        /**
         * Constructs a node evaluator.
         * @param context The current evaluation context.
         * @param statement The defined type statement to evaluate.
         */
        node_evaluator(evaluation::context& context, ast::node_statement const& statement);

        /**
         * Evaluates for the given resource.
         * @param resource The node resource to evaluate.
         */
        void evaluate(compiler::resource& resource) const;

     private:
        static std::vector<ast::parameter> _none;

        ast::node_statement const& _statement;
    };

}}}  // namespace puppet::compiler::evaluation
