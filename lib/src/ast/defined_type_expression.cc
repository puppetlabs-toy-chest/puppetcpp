#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    defined_type_expression::defined_type_expression()
    {
    }

    defined_type_expression::defined_type_expression(lexer::token_position position, ast::name name, optional<vector<parameter>> parameters, optional<vector<expression>> body) :
        _position(std::move(position)),
        _name(std::move(name)),
        _parameters(std::move(parameters)),
        _body(std::move(body))
    {
    }

    ast::name const& defined_type_expression::name() const
    {
        return _name;
    }

    optional<vector<parameter>> const& defined_type_expression::parameters() const
    {
        return _parameters;
    }

    optional<vector<expression>> const& defined_type_expression::body() const
    {
        return _body;
    }

    token_position const& defined_type_expression::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, defined_type_expression const& expr)
    {
        if (expr.name().value().empty()) {
            return os;
        }
        os << "define " << expr.name();
        if (expr.parameters()) {
            os << " (";
            pretty_print(os, expr.parameters(), ", ");
            os << ")";
        }
        os << " {";
        pretty_print(os, expr.body(), "; ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast

