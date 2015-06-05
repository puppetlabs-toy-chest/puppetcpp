/**
 * @file
 * Declares the catalog expression evaluator.
 */
#pragma once

#include "../expression_evaluator.hpp"
#include <boost/format.hpp>

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

        result_type declare_classes(ast::resource_expression const& expr);

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
