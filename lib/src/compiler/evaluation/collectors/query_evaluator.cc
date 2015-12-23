#include <puppet/compiler/evaluation/collectors/query_evaluator.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/evaluation/context.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
namespace x3 = boost::spirit::x3;

namespace puppet { namespace compiler { namespace evaluation { namespace collectors {

    query_evaluator::query_evaluator(evaluation::context& context, boost::optional<ast::query_expression> const& expression) :
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

        // Climb the remainder of the expression
        auto begin = _expression->operations.begin();
        return climb_expression(_expression->primary, 0, begin, _expression->operations.end(), resource);
    }

    bool query_evaluator::evaluate(ast::primary_query_expression const& expression, compiler::resource const& resource) const
    {
        // Handle nested expressions
        if (auto nested = boost::get<x3::forward_ast<ast::nested_query_expression>>(&expression)) {
            query_evaluator evaluator{ _context, nested->get().expression };
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
            if (auto array = attribute->value().as<values::array>()) {
                for (auto const& element : *array) {
                    if (element == expected) {
                        result = true;
                        break;
                    }
                }
            }

            // Otherwise, compare for equality
            result = result || attribute->value() == expected;
        }

        if (query.operator_ == ast::query_operator::not_equals) {
            result = !result;
        }
        return result;
    }

    bool query_evaluator::climb_expression(
        ast::primary_query_expression const& expression,
        std::uint8_t min_precedence,
        std::vector<ast::binary_query_operation>::const_iterator& begin,
        std::vector<ast::binary_query_operation>::const_iterator const& end,
        compiler::resource const& resource) const
    {
        // Evaluate the left-hand side of the expression
        auto result = evaluate(expression, resource);

        // Climb the binary operations based on operator precedence
        uint8_t precedence;
        while (begin != end && (precedence = get_precedence(begin->operator_)) >= min_precedence)
        {
            auto& operation = *begin;
            ++begin;

            // Attempt short circuiting
            if ((operation.operator_ == ast::binary_query_operator::logical_and && !result) ||
                (operation.operator_ == ast::binary_query_operator::logical_or && result)) {
                begin = end;
                return result;
            }

            // Recurse and climb the expression
            uint8_t next_precedence = precedence + (is_right_associative(operation.operator_) ? static_cast<uint8_t>(0) : static_cast<uint8_t>(1));
            result = climb_expression(operation.operand, next_precedence, begin, end, resource);
        }
        return result;
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

}}}}  // namespace puppet::compiler::evaluation::collectors
