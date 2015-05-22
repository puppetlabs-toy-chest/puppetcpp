#include <puppet/ast/expression_def.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    struct position_visitor : boost::static_visitor<lexer::position const&>
    {
        result_type operator()(boost::blank const&) const
        {
            return default_position;
        }

        result_type operator()(basic_expression const& expr) const
        {
            return boost::apply_visitor(*this, expr);
        }

        result_type operator()(control_flow_expression const& expr) const
        {
            return boost::apply_visitor(*this, expr);
        }

        result_type operator()(catalog_expression const& expr) const
        {
            return boost::apply_visitor(*this, expr);
        }

        result_type operator()(function_call_expression const& expr) const
        {
            return expr.position();
        }

        result_type operator()(resource_expression const& expr) const
        {
            return expr.position();
        }

        result_type operator()(resource_defaults_expression const& expr) const
        {
            return expr.position();
        }

        result_type operator()(collection_expression const& expr) const
        {
            return expr.position();
        }

        result_type operator()(resource_override_expression const& expr) const
        {
            return get_position(expr.reference);
        }

        result_type operator()(postfix_expression const& expr) const
        {
            return expr.position();
        }

        result_type operator()(expression const& expr) const
        {
            return expr.position();
        }

        template <typename T>
        result_type operator()(T const& element) const
        {
            return element.position;
        }

    private:
        static lexer::position default_position;
    };

    lexer::position const& get_position(primary_expression const& expr)
    {
        return boost::apply_visitor(position_visitor(), expr);
    }

    lexer::position position_visitor::default_position;


    bool is_blank(primary_expression const& expr)
    {
        return expr.which() == 0;
    }

    ostream& operator<<(ostream& os, unary_operator op)
    {
        switch (op) {
            case unary_operator::none:
                break;

            case unary_operator::negate:
                os << '-';
                break;

            case unary_operator::logical_not:
                os << '!';
                break;

            case unary_operator::splat:
                os << '*';
                break;

            default:
                throw runtime_error("invalid unary operator.");
        }
        return os;
    }

    unary_expression::unary_expression() :
        op(unary_operator::none)
    {
    }

    unary_expression::unary_expression(lexer::position position, unary_operator op, primary_expression operand) :
        position(rvalue_cast(position)),
        op(op),
        operand(rvalue_cast(operand))
    {
    }

    ostream& operator<<(ostream& os, unary_expression const& expr)
    {
        if (expr.op == unary_operator::none) {
            os << expr.operand;
            return os;
        }
        os << expr.op << expr.operand;
        return os;
    }

    ostream& operator<<(ostream& os, binary_operator op)
    {
        switch (op) {
            case binary_operator::none:
                break;

            case binary_operator::in:
                os << "in";
                break;
            case binary_operator::match:
                os << "=~";
                break;
            case binary_operator::not_match:
                os << "!~";
                break;
            case binary_operator::multiply:
                os << "*";
                break;
            case binary_operator::divide:
                os << "/";
                break;
            case binary_operator::modulo:
                os << "%";
                break;
            case binary_operator::plus:
                os << "+";
                break;
            case binary_operator::minus:
                os << "-";
                break;
            case binary_operator::left_shift:
                os << "<<";
                break;
            case binary_operator::right_shift:
                os << ">>";
                break;
            case binary_operator::equals:
                os << "==";
                break;
            case binary_operator::not_equals:
                os << "!=";
                break;
            case binary_operator::greater_than:
                os << ">";
                break;
            case binary_operator::greater_equals:
                os << ">=";
                break;
            case binary_operator::less_than:
                os << "<";
                break;
            case binary_operator::less_equals:
                os << "<=";
                break;
            case binary_operator::logical_and:
                os << "and";
                break;
            case binary_operator::logical_or:
                os << "or";
                break;
            case binary_operator::assignment:
                os << '=';
                break;
            case binary_operator::in_edge:
                os << "->";
                break;
            case binary_operator::in_edge_subscribe:
                os << "~>";
                break;
            case binary_operator::out_edge:
                os << "<-";
                break;
            case binary_operator::out_edge_subscribe:
                os << "<~";
                break;
            default:
                throw runtime_error("invalid binary operator.");
        }
        return os;
    }

    binary_expression::binary_expression() :
        op(binary_operator::none)
    {
    }

    binary_expression::binary_expression(binary_operator op, primary_expression operand) :
        op(op),
        operand(rvalue_cast(operand))
    {
    }

    lexer::position const& binary_expression::position() const
    {
        return get_position(operand);
    }

    ostream& operator<<(ostream& os, binary_expression const& expr)
    {
        if (expr.op == binary_operator::none) {
            return os;
        }
        os << " " << expr.op << " " << expr.operand;
        return os;
    }

    expression::expression()
    {
    }

    expression::expression(primary_expression primary, vector<binary_expression> binary) :
        primary(rvalue_cast(primary)),
        binary(rvalue_cast(binary))
    {
    }

    lexer::position const& expression::position() const
    {
        return get_position(primary);
    }

    ostream& operator<<(ostream& os, expression const& expr)
    {
        os << "(" << expr.primary;
        for (auto const& binary : expr.binary) {
            os << binary;
        }
        os << ")";
        return os;
    }

}}  // namespace puppet::ast
