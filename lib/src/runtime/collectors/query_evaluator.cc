#include <puppet/runtime/collectors/query_evaluator.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace collectors {

    query_evaluator::query_evaluator(expression_evaluator& evaluator, boost::optional<ast::query> const& query) :
        _evaluator(evaluator),
        _query(query)
    {
    }

    bool query_evaluator::evaluate(runtime::resource const& resource) const
    {
        // Accept all if not given a query to evaluate
        if (!_query) {
            return true;
        }

        // Evaluate the primary expression
        auto result = evaluate(_query->primary(), resource);

        // Climb the remainder of the expression
        auto begin = _query->binary().begin();
        climb_expression(result, 0, begin, _query->binary().end(), resource);
        return result;
    }

    bool query_evaluator::evaluate(ast::primary_attribute_query const& expression, runtime::resource const& resource) const
    {
        // Handle nested expressions
        if (auto nested = boost::get<ast::query>(&expression)) {
            query_evaluator evaluator(_evaluator, *nested);
            return evaluator.evaluate(resource);
        }

        // Otherwise, this should be an attribute query
        auto& query = boost::get<ast::attribute_query>(expression);

        // Evaluate the expected value
        auto expected = _evaluator.evaluate(query.value());

        // If the query is on the title, search the resource's title
        bool result = false;
        if (query.attribute().value() == "title") {
            if (auto str = as<string>(expected)) {
                result = resource.type().title() == *str;
            }
        } else {
            // If the attribute doesn't exist, return false
            auto attribute = resource.get(query.attribute().value());
            if (!attribute) {
                return false;
            }

            // If the attribute's value is an array, first check for containment
            if (auto array = as<values::array>(*attribute->value())) {
                for (auto const& element : *array) {
                    if (equals(element, expected)) {
                        result = true;
                        break;
                    }
                }
            }

            // Otherwise, compare for equality
            result = result || equals(*attribute->value(), expected);
        }

        if (query.op() == ast::attribute_query_operator::not_equals) {
            result = !result;
        }
        return result;
    }

    void query_evaluator::climb_expression(
        bool& result,
        std::uint8_t min_precedence,
        std::vector<ast::binary_query_expression>::const_iterator& begin,
        std::vector<ast::binary_query_expression>::const_iterator const& end,
        runtime::resource const& resource) const
    {
        // This member implements precedence climbing for binary attribute expressions
        uint8_t precedence;
        while (begin != end && (precedence = get_precedence(begin->op())) >= min_precedence)
        {
            auto op = begin->op();
            auto& operand = begin->operand();
            ++begin;

            // Attempt short circuiting
            if ((op == ast::binary_query_operator::logical_and && !result) ||
                (op == ast::binary_query_operator::logical_or && result)) {
                begin = end;
                return;
            }

            // Evaluate the right side
            bool right = evaluate(operand, resource);

            // Recurse and climb the expression
            uint8_t next_precedence = precedence + (is_right_associative(op) ? static_cast<uint8_t>(0) : static_cast<uint8_t>(1));
            climb_expression(right, next_precedence, begin, end, resource);

            // Can assign directly to result thanks to the short-circuiting logic above
            result = right;
        }
    }

    uint8_t query_evaluator::get_precedence(ast::binary_query_operator op)
    {
        // Return the precedence (low to high)
        switch (op) {
            case ast::binary_query_operator::logical_or:
                return 1;

            case ast::binary_query_operator::logical_and:
                return 2;

            default:
                break;
        }

        throw runtime_error("invalid binary query operator.");
    }

    bool query_evaluator::is_right_associative(ast::binary_query_operator op)
    {
        // Currently all operators are left-associative
        return false;
    }

}}}  // namespace puppet::runtime::collectors
