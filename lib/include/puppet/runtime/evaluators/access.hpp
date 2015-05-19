/**
 * @file
 * Declares the access expression evaluator.
 */
#pragma once

#include "../expression_evaluator.hpp"
#include "../../ast/access_expression.hpp"
#include <tuple>
#include <limits>
#include <boost/format.hpp>

namespace puppet { namespace runtime { namespace evaluators {

    /**
     * Implements the access expression evaluator.
     */
    struct access_expression_evaluator : boost::static_visitor<values::value>
    {
        /**
         * Constructs a access expression evaluator.
         * @param evaluator The expression evaluator to use.
         * @param expression The access expression to evaluate.
         */
        access_expression_evaluator(expression_evaluator& evaluator, ast::access_expression const& expression);

        /**
         * Evaluates a access expression.
         * @param target The target value to evaluate the access expression against.
         * @return Returns the evaluated value.
         */
        result_type evaluate(values::value const& target);

     private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;
        result_type operator()(std::string const& target);
        result_type operator()(values::array const& target);
        result_type operator()(values::hash const& target);
        result_type operator()(values::type const& target);
        result_type operator()(types::integer const& type);
        result_type operator()(types::floating const& type);
        result_type operator()(types::string const& type);
        result_type operator()(types::regexp const& type);
        result_type operator()(types::enumeration const& type);
        result_type operator()(types::pattern const& type);
        result_type operator()(types::array const& type);
        result_type operator()(types::hash const& type);
        result_type operator()(types::tuple const& type);
        result_type operator()(types::optional const& type);
        result_type operator()(types::type const& type);
        result_type operator()(types::structure const& type);
        result_type operator()(types::variant const& type);
        result_type operator()(types::resource const& type);
        result_type operator()(types::klass const& type);

        template <typename T>
        result_type operator()(T const& target)
        {
            throw evaluation_exception(_expression.position(), (boost::format("access expression is not supported for %1%.") % values::get_type(target)).str());
        }

        template <typename Value, typename Type>
        std::tuple<Value, Value> get_range(bool accept_range = false, size_t start_index = 0) const
        {
            using namespace std;
            using namespace puppet::runtime::values;

            // Check for Integer range first
            if (accept_range && _arguments.size() > start_index) {
                auto type_ptr = as<values::type>(_arguments[start_index]);
                if (type_ptr) {
                    auto integer_ptr = boost::get<types::integer>(type_ptr);
                    if (integer_ptr) {
                        return make_tuple(integer_ptr->from(), integer_ptr->to());
                    }
                }
            }

            // Get the from argument
            Value from = numeric_limits<Value>::min();
            if (_arguments.size() > start_index) {
                auto& argument = _arguments[start_index];

                if (!is_default(argument)) {
                    // Try int64_t first; this allows either Integer or Floats when Value is floating point
                    auto ptr = as<int64_t>(argument);
                    if (ptr) {
                        from = static_cast<Value>(*ptr);
                    } else {
                        auto value_ptr = as<Value>(argument);
                        if (!value_ptr) {
                            throw evaluation_exception(_positions[start_index], (boost::format("expected parameter to be %1% but found %2%.") % Type::name() % get_type(argument)).str());
                        }
                        from = *value_ptr;
                    }
                }
            }
            // Get the to argument
            Value to = numeric_limits<Value>::max();
            if (_arguments.size() > (start_index + 1)) {
                auto& argument = _arguments[start_index + 1];

                if (!is_default(argument)) {
                    // Try int64_t first; this allows either Integer or Floats when Value is floating point
                    auto ptr = as<int64_t>(argument);
                    if (ptr) {
                        to = static_cast<Value>(*ptr);
                    } else {
                        auto value_ptr = as<Value>(argument);
                        if (!value_ptr) {
                            throw evaluation_exception(_positions[start_index], (boost::format("expected parameter to be %1% but found %2%.") % Type::name() % get_type(argument)).str());
                        }
                        to = *value_ptr;
                    }
                }
            }
            return make_tuple(from, to);
        }

        expression_evaluator& _evaluator;
        ast::access_expression const& _expression;
        values::array _arguments;
        std::vector<lexer::token_position> _positions;
    };

}}}  // puppet::runtime::evaluators
