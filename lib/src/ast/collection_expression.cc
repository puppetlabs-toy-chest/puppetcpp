#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
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

    attribute_query::attribute_query() :
        _op(attribute_query_operator::none)
    {
    }

    attribute_query::attribute_query(name attribute, attribute_query_operator op, basic_expression value) :
        _attribute(rvalue_cast(attribute)),
        _op(op),
        _value(rvalue_cast(value))
    {
    }

    name const& attribute_query::attribute() const
    {
        return _attribute;
    }

    attribute_query_operator attribute_query::op() const
    {
        return _op;
    }

    basic_expression const& attribute_query::value() const
    {
        return _value;
    }

    lexer::position const& attribute_query::position() const
    {
        return _attribute.position();
    }

    ostream& operator<<(ostream& os, ast::attribute_query const& query)
    {
        if (query.attribute().value().empty()) {
            return os;
        }
        os << query.attribute() << " " << query.op() << " " << query.value();
        return os;
    }

    struct query_position_visitor : boost::static_visitor<lexer::position const&>
    {
        result_type operator()(ast::attribute_query const& query) const
        {
            return query.position();
        }

        result_type operator()(ast::query const& query) const
        {
            return query.position();
        }
    };

    lexer::position const& get_query_position(ast::primary_attribute_query const& query)
    {
        return boost::apply_visitor(query_position_visitor(), query);
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

    binary_query_expression::binary_query_expression(binary_query_operator op, primary_attribute_query operand) :
        _op(op),
        _operand(rvalue_cast(operand))
    {
    }

    binary_query_operator binary_query_expression::op() const
    {
        return _op;
    }

    primary_attribute_query const& binary_query_expression::operand() const
    {
        return _operand;
    }

    lexer::position const& binary_query_expression::position() const
    {
        return get_query_position(_operand);
    }

    ostream& operator<<(ostream& os, binary_query_expression const& expr)
    {
        if (expr.op() == binary_query_operator::none) {
            return os;
        }

        os << " " << expr.op() << " " << expr.operand();
        return os;
    }

    query::query()
    {
    }

    query::query(primary_attribute_query primary, vector<binary_query_expression> binary) :
        _primary(rvalue_cast(primary)),
        _binary(rvalue_cast(binary))
    {
    }

    primary_attribute_query const& query::primary() const
    {
        return _primary;
    }

    vector<binary_query_expression> const& query::binary() const
    {
        return _binary;
    }

    lexer::position const& query::position() const
    {
        return get_query_position(_primary);
    }

    ostream& operator<<(ostream& os, ast::query const& query)
    {
        if (!query.binary().empty()) {
            os << '(';
        }
        os << query.primary();
        for (auto const& binary : query.binary()) {
            os << binary;
        }
        if (!query.binary().empty()) {
            os << ')';
        }
        return os;
    }

    collection_expression::collection_expression() :
        _kind(collection_kind::none)
    {
    }

    collection_expression::collection_expression(collection_kind kind, ast::type type, optional<ast::query> query) :
        _kind(kind),
        _type(rvalue_cast(type)),
        _query(rvalue_cast(query))
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

    optional<query> const& collection_expression::query() const
    {
        return _query;
    }

    lexer::position const& collection_expression::position() const
    {
        return _type.position();
    }

    ostream& operator<<(ostream& os, collection_expression const& expr)
    {
        if (expr.kind() == collection_kind::none) {
            return os;
        }
        os << expr.type() << " " << (expr.kind() == collection_kind::all ? "<| " : "<<| ");
        if (expr.query()) {
            os << *expr.query();
        }
        os << (expr.kind() == collection_kind::all ? " |>" : " |>>");
        return os;
    }

}}  // namespace puppet::ast

