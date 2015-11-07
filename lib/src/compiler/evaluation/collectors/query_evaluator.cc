#include <puppet/compiler/evaluation/collectors/query_evaluator.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/evaluation/context.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
namespace x3 = boost::spirit::x3;

namespace puppet { namespace compiler { namespace evaluation { namespace collectors {

    query_evaluator::query_evaluator(evaluation::context& context, boost::optional<ast::collector_query_expression> const& expression) :
        _context(context),
        _expression(expression)
    {
    }

    bool query_evaluator::evaluate(compiler::resource const& resource) const
    {
        // Accept all if not given a query to evaluate
        if (!_expression) {
            return true;
        }

        // Evaluate the primary expression
        auto result = evaluate(_expression->primary, resource);

        // Climb the remainder of the expression
        auto begin = _expression->remainder.begin();
        climb_expression(result, 0, begin, _expression->remainder.end(), resource);
        return result;
    }

    bool query_evaluator::evaluate(ast::attribute_query_expression const& expression, compiler::resource const& resource) const
    {
        // Handle nested expressions
        if (auto nested = boost::get<x3::forward_ast<ast::collector_query_expression>>(&expression)) {
            query_evaluator evaluator{ _context, nested->get() };
            return evaluator.evaluate(resource);
        }

        evaluation::evaluator evaluator{ _context };

        // Otherwise, this should be an attribute query
        auto& query = boost::get<ast::attribute_query>(expression);

        // Evaluate the expected value
        auto expected = evaluator.evaluate(query.value);

        // If the query is on the title, search the resource's title
        bool result = false;
        if (query.attribute.value == "title") {
            if (auto str = expected.as<string>()) {
                result = resource.type().title() == *str;
            }
        } else {
            // If the attribute doesn't exist, return false
            auto attribute = resource.get(query.attribute.value);
            if (!attribute) {
                return false;
            }

            // If the attribute's value is an array, first check for containment
            if (auto array = attribute->shared_value()->as<values::array>()) {
                for (auto const& element : *array) {
                    if (element == expected) {
                        result = true;
                        break;
                    }
                }
            }

            // Otherwise, compare for equality
            result = result || *attribute->shared_value() == expected;
        }

        if (query.oper == ast::attribute_query_operator::not_equals) {
            result = !result;
        }
        return result;
    }

    void query_evaluator::climb_expression(
        bool& result,
        std::uint8_t min_precedence,
        std::vector<ast::binary_attribute_query>::const_iterator& begin,
        std::vector<ast::binary_attribute_query>::const_iterator const& end,
        compiler::resource const& resource) const
    {
        // This member implements precedence climbing for binary attribute expressions
        uint8_t precedence;
        while (begin != end && (precedence = get_precedence(begin->oper)) >= min_precedence)
        {
            auto oper = begin->oper;
            auto& operand = begin->operand;
            ++begin;

            // Attempt short circuiting
            if ((oper == ast::binary_query_operator::logical_and && !result) ||
                (oper == ast::binary_query_operator::logical_or && result)) {
                begin = end;
                return;
            }

            // Evaluate the right side
            bool right = evaluate(operand, resource);

            // Recurse and climb the expression
            uint8_t next_precedence = precedence + (is_right_associative(oper) ? static_cast<uint8_t>(0) : static_cast<uint8_t>(1));
            climb_expression(right, next_precedence, begin, end, resource);

            // Can assign directly to result thanks to the short-circuiting logic above
            result = right;
        }
    }

    uint8_t query_evaluator::get_precedence(ast::binary_query_operator oper)
    {
        // Return the precedence (low to high)
        switch (oper) {
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

}}}}  // namespace puppet::compiler::evaluation::collectors
