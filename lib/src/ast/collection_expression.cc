#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    ostream& operator<<(ostream& os, attribute_query_operator const& op)
    {
        switch (op) {
            case attribute_query_operator::equals:
                os << "==";
                break;

            case attribute_query_operator::not_equals:
                os << "!=";
                break;

            default:
                throw runtime_error("invalid attribute operator.");
        }
        return os;
    }

    query::query() :
        _op(attribute_query_operator::none)
    {
    }

    query::query(name attribute, attribute_query_operator op, basic_expression value) :
        _attribute(std::move(attribute)),
        _op(op),
        _value(std::move(value))
    {
    }

    name const& query::attribute() const
    {
        return _attribute;
    }

    attribute_query_operator query::op() const
    {
        return _op;
    }

    basic_expression const& query::value() const
    {
        return _value;
    }

    token_position const& query::position() const
    {
        return _attribute.position();
    }

    ostream& operator<<(ostream& os, ast::query const& query)
    {
        if (query.attribute().value().empty()) {
            return os;
        }
        os << query.attribute() << " " << query.op() << " " << query.value();
        return os;
    }

    ostream& operator<<(ostream& os, binary_query_operator const& op)
    {
        switch (op) {
            case binary_query_operator::logical_and:
                os << "and";
                break;

            case binary_query_operator::logical_or:
                os << "or";
                break;

            default:
                throw runtime_error("invalid attribute operator.");
        }
        return os;
    }

    binary_query_expression::binary_query_expression() :
        _op(binary_query_operator::none)
    {
    }

    binary_query_expression::binary_query_expression(binary_query_operator op, query operand) :
        _op(op),
        _operand(std::move(operand))
    {
    }

    binary_query_operator binary_query_expression::op() const
    {
        return _op;
    }

    query const& binary_query_expression::operand() const
    {
        return _operand;
    }

    token_position const& binary_query_expression::position() const
    {
        return _operand.position();
    }

    ostream& operator<<(ostream& os, binary_query_expression const& expr)
    {
        if (expr.op() == binary_query_operator::none) {
            return os;
        }

        os << expr.op() << " " << expr.operand();
        return os;
    }

    collection_expression::collection_expression() :
        _kind(collection_kind::none)
    {
    }

    collection_expression::collection_expression(collection_kind kind, ast::type type, optional<query> first, vector<binary_query_expression> remainder) :
        _kind(kind),
        _type(std::move(type)),
        _first(std::move(first)),
        _remainder(std::move(remainder))
    {
    }

    collection_kind collection_expression::kind() const
    {
        return _kind;
    }

    ast::type const& collection_expression::type() const
    {
        return _type;
    }

    optional<query> const& collection_expression::first() const
    {
        return _first;
    }

    vector<binary_query_expression> const& collection_expression::remainder() const
    {
        return _remainder;
    }

    token_position const& collection_expression::position() const
    {
        return _type.position();
    }

    ostream& operator<<(ostream& os, collection_expression const& expr)
    {
        if (expr.kind() == collection_kind::none) {
            return os;
        }
        os << expr.type() << " " << (expr.kind() == collection_kind::all ? "<| " : "<<| ");
        if (expr.first()) {
            os << *expr.first();
        }
        for (auto const& bexpr : expr.remainder()) {
            os << bexpr;
        }
        os << (expr.kind() == collection_kind::all ? " |>" : " |>>");
        return os;
    }

}}  // namespace puppet::ast

