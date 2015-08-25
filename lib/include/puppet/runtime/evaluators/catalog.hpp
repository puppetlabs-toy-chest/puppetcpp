/**
 * @file
 * Declares the catalog expression evaluator.
 */
#pragma once

#include "../expression_evaluator.hpp"
#include <boost/format.hpp>
#include <vector>
#include <memory>
#include <tuple>
#include <unordered_set>

namespace puppet { namespace runtime { namespace evaluators {

    /**
     * Implements the catalog expression evaluator.
     */
    struct catalog_expression_evaluator : boost::static_visitor<values::value>
    {
        /**
         * Constructs a catalog expression evaluator.
         * @param evaluator The expression evaluator to use.
         * @param expression The catalog expression to evaluate.
         */
        catalog_expression_evaluator(expression_evaluator& evaluator, ast::catalog_expression const& expression);

        /**
         * Evaluates the catalog expression.
         * @return Returns the evaluated value.
         */
        result_type evaluate();

     private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;
        result_type operator()(ast::resource_expression const& expr);
        result_type operator()(ast::resource_defaults_expression const& expr);
        result_type operator()(ast::resource_override_expression const& expr);
        result_type operator()(ast::class_definition_expression const& expr);
        result_type operator()(ast::defined_type_expression const& expr);
        result_type operator()(ast::node_definition_expression const& expr);
        result_type operator()(ast::collection_expression const& expr);

        static bool is_default_expression(ast::primary_expression const& expr);
        ast::resource_body const* find_default_body(ast::resource_expression const& expr);
        std::vector<std::pair<ast::attribute_operator, std::shared_ptr<attribute>>> evaluate_attributes(
            bool is_class,
            boost::optional<std::vector<ast::attribute_expression>> const& expressions);
        void validate_attribute(lexer::position const& position, std::string const& name, values::value& value);
        void splat_attribute(
            std::vector<std::pair<ast::attribute_operator, std::shared_ptr<attribute>>>& attributes,
            std::unordered_set<std::string>& names,
            ast::attribute_expression const& attribute);
        std::vector<resource*> create_resources(
            bool is_class,
            std::string const& type_name,
            ast::resource_expression const& expression,
            std::vector<std::pair<ast::attribute_operator, std::shared_ptr<attribute>>> const& default_attributes);
        void set_attributes(runtime::resource& resource, std::vector<std::pair<ast::attribute_operator, std::shared_ptr<attribute>>> const& attributes);

        template <typename T>
        bool for_each(values::value& parameter, std::function<void(T&)> const& callback)
        {
            using namespace puppet::runtime::values;

            if (as<T>(parameter)) {
                auto casted = mutate_as<T>(parameter);
                callback(casted);
                return true;
            }
            if (as<values::array>(parameter)) {
                // For arrays, recurse on each element
                auto array = mutate_as<values::array>(parameter);
                for (auto& element : array) {
                    if (!for_each<T>(element, callback)) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }

        expression_evaluator& _evaluator;
        ast::catalog_expression const& _expression;
    };

}}}  // puppet::runtime::evaluators
