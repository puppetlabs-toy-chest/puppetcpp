#include "expression_def.hpp"
#include "utility.hpp"

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    class_definition_expression::class_definition_expression()
    {
    }


    class_definition_expression::class_definition_expression(token_position position, ast::name name, optional<vector<parameter>> parameters, optional<ast::name> parent, optional<vector<expression>> body) :
        _position(position),
        _name(std::move(name)),
        _parameters(std::move(parameters)),
        _parent(std::move(parent)),
        _body(std::move(body))
    {
    }

    ast::name const& class_definition_expression::name() const
    {
        return _name;
    }

    ast::name& class_definition_expression::name()
    {
        return _name;
    }

    optional<vector<parameter>> const& class_definition_expression::parameters() const
    {
        return _parameters;
    }

    optional<vector<parameter>>& class_definition_expression::parameters()
    {
        return _parameters;
    }

    optional<ast::name> const& class_definition_expression::parent() const
    {
        return _parent;
    }

    optional<ast::name>& class_definition_expression::parent()
    {
        return _parent;
    }

    optional<vector<expression>> const& class_definition_expression::body() const
    {
        return _body;
    }

    optional<vector<expression>>& class_definition_expression::body()
    {
        return _body;
    }

    token_position const& class_definition_expression::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, class_definition_expression const& expr)
    {
        if (expr.name().value().empty()) {
            return os;
        }

        os << "class " << expr.name();
        if (expr.parameters()) {
            os << " (";
            pretty_print(os, expr.parameters(), ", ");
            os << ")";
        }
        if (expr.parent()) {
            os << " inherits " << *expr.parent();
        }
        os << " {";
        pretty_print(os, expr.body(), "; ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast

