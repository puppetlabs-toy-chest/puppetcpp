#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/runtime/evaluators/primary.hpp>
#include <puppet/runtime/operators/assignment.hpp>
#include <puppet/runtime/operators/divide.hpp>
#include <puppet/runtime/operators/equals.hpp>
#include <puppet/runtime/operators/greater.hpp>
#include <puppet/runtime/operators/greater_equal.hpp>
#include <puppet/runtime/operators/in.hpp>
#include <puppet/runtime/operators/left_shift.hpp>
#include <puppet/runtime/operators/less.hpp>
#include <puppet/runtime/operators/less_equal.hpp>
#include <puppet/runtime/operators/logical_and.hpp>
#include <puppet/runtime/operators/logical_or.hpp>
#include <puppet/runtime/operators/match.hpp>
#include <puppet/runtime/operators/minus.hpp>
#include <puppet/runtime/operators/modulo.hpp>
#include <puppet/runtime/operators/multiply.hpp>
#include <puppet/runtime/operators/not_equals.hpp>
#include <puppet/runtime/operators/not_match.hpp>
#include <puppet/runtime/operators/plus.hpp>
#include <puppet/runtime/operators/right_shift.hpp>
#include <puppet/runtime/definition_scanner.hpp>
#include <puppet/runtime/dispatcher.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    evaluation_exception::evaluation_exception(lexer::position position, string const& message) :
        runtime_error(message),
        _position(rvalue_cast(position))
    {
    }

    lexer::position const& evaluation_exception::position() const
    {
        return _position;
    }

    expression_evaluator::expression_evaluator(shared_ptr<compiler::context> compilation_context, runtime::context& evaluation_context) :
        _compilation_context(rvalue_cast(compilation_context)),
        _evaluation_context(evaluation_context)
    {
        if (!_compilation_context) {
            throw runtime_error("expected a compilation context.");
        }
    }

    runtime::context& expression_evaluator::context()
    {
        return _evaluation_context;
    }

    runtime::catalog* expression_evaluator::catalog()
    {
        return _evaluation_context.catalog();
    }

    logging::logger& expression_evaluator::logger()
    {
        return _compilation_context->logger();
    }

    shared_ptr<string> const& expression_evaluator::path() const
    {
        return _compilation_context->path();
    }

    void expression_evaluator::warn(lexer::position const& position, string const& message)
    {
        _compilation_context->log(logging::level::warning, position, message);
    }

    void expression_evaluator::evaluate()
    {
        auto& tree = _compilation_context->tree();
        if (!tree.body()) {
            return;
        }

        // Scan the tree for definitions
        if (catalog()) {
            definition_scanner scanner{ *catalog() };
            scanner.scan(_compilation_context);
        }

        for (auto& expression : *tree.body()) {
            // Top level expressions must be productive
            evaluate(expression, true);
        }
    }

    value expression_evaluator::evaluate(ast::expression const& expr, bool productive)
    {
        if (productive && !is_productive(expr)) {
            throw evaluation_exception(expr.position(), "unproductive expressions may only appear last in a block.");
        }

        // Evaluate the primary expression
        auto result = evaluate(expr.primary());

        // Climb the remainder of the expression
        auto begin = expr.binary().begin();
        climb_expression(result, expr.position(), 0, begin, expr.binary().end());

        return result;
    }

    value expression_evaluator::evaluate(ast::primary_expression const& expr)
    {
        evaluators::primary_expression_evaluator evaluator(*this, expr);
        return evaluator.evaluate();
    }

    boost::optional<values::array> expression_evaluator::unfold(ast::expression const& expr, value& result)
    {
        // An unfold expression is always unary with no further expressions
        if (!expr.binary().empty()) {
            return boost::none;
        }
        // Unfold the first expression
        return unfold(expr.primary(), result);
    }

    boost::optional<values::array> expression_evaluator::unfold(ast::primary_expression const& expression, value& evaluated)
    {
        // Determine if the given expression is a unary splat of an array
        auto unary = boost::get<ast::unary_expression>(&expression);
        if (unary && unary->op() == ast::unary_operator::splat && as<values::array>(evaluated)) {
            return mutate_as<values::array>(evaluated);
        }

        // Try for nested expression
        auto nested = boost::get<ast::expression>(&expression);
        if (nested) {
            return unfold(*nested, evaluated);
        }
        return boost::none;
    }

    bool expression_evaluator::is_match(value& actual, lexer::position const& actual_position, value& expected, lexer::position const& expected_position)
    {
        // If the expected value is a regex, use match
        auto regex = as<values::regex>(expected);
        if (regex) {
            // Only match against strings
            if (as<string>(actual)) {
                operators::binary_context context(*this, actual, actual_position, expected, expected_position);
                if (is_truthy(operators::match()(context))) {
                    return true;
                }
            }
            return false;
        }

        // Otherwise, use equals
        return equals(actual, expected);
    }

    bool expression_evaluator::is_productive(ast::expression const& expr)
    {
        // Check if the primary expression itself is productive
        if (is_productive(expr.primary())) {
            return true;
        }

        // Expressions followed by an assignment or relationship operator are productive
        for (auto const& binary : expr.binary()) {
            if (binary.op() == ast::binary_operator::assignment ||
                binary.op() == ast::binary_operator::in_edge ||
                binary.op() == ast::binary_operator::in_edge_subscribe ||
                binary.op() == ast::binary_operator::out_edge ||
                binary.op() == ast::binary_operator::out_edge_subscribe) {
                return true;
            }
        }

        return false;
    }

    bool expression_evaluator::is_productive(ast::primary_expression const& expr)
    {
        // Check for recursive primary expression
        if (auto ptr = boost::get<ast::expression>(&expr)) {
            if (is_productive(*ptr)) {
                return true;
            }
        }

        // Check for unary expression
        if (auto ptr = boost::get<ast::unary_expression>(&expr)) {
            if (is_productive(ptr->operand())) {
                return true;
            }
        }

        // Catalog and control flow expressions are productive
        if (boost::get<ast::catalog_expression>(&expr) ||
            boost::get<ast::control_flow_expression>(&expr)) {
            return true;
        }

        // Postfix method calls are productive
        if (auto ptr = boost::get<ast::postfix_expression>(&expr)) {
            if (is_productive(ptr->primary())) {
                return true;
            }
            for (auto const& subexpression : ptr->subexpressions()) {
                if (boost::get<ast::method_call_expression>(&subexpression)) {
                    return true;
                }
            }
        }
        return false;
    }

    void expression_evaluator::climb_expression(
        value& left,
        lexer::position const& left_position,
        uint8_t min_precedence,
        vector<ast::binary_expression>::const_iterator& begin,
        vector<ast::binary_expression>::const_iterator const& end)
    {
        // This member implements precedence climbing for binary expressions
        uint8_t precedence;
        while (begin != end && (precedence = get_precedence(begin->op())) >= min_precedence)
        {
            auto op = begin->op();
            auto& operand = begin->operand();
            auto right_position = begin->position();
            ++begin;

            // If the operator is a logical and/or operator, attempt short circuiting
            if ((op == ast::binary_operator::logical_and && !is_truthy(left)) ||
                (op == ast::binary_operator::logical_or && is_truthy(left))) {
                left = op == ast::binary_operator::logical_or;
                begin = end;
                return;
            }

            // Evaluate the right side
            value right = evaluate(operand);

            // Recurse and climb the expression
            uint8_t next_precedence = precedence + (is_right_associative(op) ? static_cast<uint8_t>(0) : static_cast<uint8_t>(1));
            climb_expression(right, right_position, next_precedence, begin, end);

            // Evaluate this part of the expression
            evaluate(left, left_position, op, right, right_position);
        }
    }

    void expression_evaluator::evaluate(
        value& left,
        lexer::position const& left_position,
        ast::binary_operator op,
        value& right,
        lexer::position& right_position)
    {
        static const unordered_map<ast::binary_operator, function<values::value(operators::binary_context&)>> binary_operators = {
            { ast::binary_operator::assignment,         operators::assignment() },
            { ast::binary_operator::divide,             operators::divide() },
            { ast::binary_operator::equals,             operators::equals() },
            { ast::binary_operator::greater_than,       operators::greater() },
            { ast::binary_operator::greater_equals,     operators::greater_equal() },
            { ast::binary_operator::in,                 operators::in() },
            { ast::binary_operator::less_than,          operators::less() },
            { ast::binary_operator::less_equals,        operators::less_equal() },
            { ast::binary_operator::left_shift,         operators::left_shift() },
            { ast::binary_operator::logical_and,        operators::logical_and() },
            { ast::binary_operator::logical_or,         operators::logical_or() },
            { ast::binary_operator::match,              operators::match() },
            { ast::binary_operator::minus,              operators::minus() },
            { ast::binary_operator::modulo,             operators::modulo() },
            { ast::binary_operator::multiply,           operators::multiply() },
            { ast::binary_operator::not_equals,         operators::not_equals() },
            { ast::binary_operator::not_match,          operators::not_match() },
            { ast::binary_operator::plus,               operators::plus() },
            { ast::binary_operator::right_shift,        operators::right_shift() }
        };

        auto it = binary_operators.find(op);
        if (it == binary_operators.end()) {
            throw evaluation_exception(left_position, (boost::format("unspported binary operator '%1%' in binary expression.") % op).str());
        }

        operators::binary_context context(*this, left, left_position, right, right_position);
        left = it->second(context);
    }

    uint8_t expression_evaluator::get_precedence(ast::binary_operator op)
    {
        // Return the precedence (low to high)
        switch (op) {
            case ast::binary_operator::in_edge:
            case ast::binary_operator::in_edge_subscribe:
            case ast::binary_operator::out_edge:
            case ast::binary_operator::out_edge_subscribe:
                return 1;

            case ast::binary_operator::assignment:
                return 2;

            case ast::binary_operator::logical_or:
                return 3;

            case ast::binary_operator::logical_and:
                return 4;

            case ast::binary_operator::greater_than:
            case ast::binary_operator::greater_equals:
            case ast::binary_operator::less_than:
            case ast::binary_operator::less_equals:
                return 5;

            case ast::binary_operator::equals:
            case ast::binary_operator::not_equals:
                return 6;

            case ast::binary_operator::left_shift:
            case ast::binary_operator::right_shift:
                return 7;

            case ast::binary_operator::plus:
            case ast::binary_operator::minus:
                return 8;

            case ast::binary_operator::multiply:
            case ast::binary_operator::divide:
            case ast::binary_operator::modulo:
                return 9;

            case ast::binary_operator::match:
            case ast::binary_operator::not_match:
                return 10;

            case ast::binary_operator::in:
                return 11;

            default:
                break;
        }

        throw runtime_error("invalid binary operator.");
    }

    bool expression_evaluator::is_right_associative(ast::binary_operator op)
    {
        return op == ast::binary_operator::assignment;
    }

}}  // namespace puppet::runtime
