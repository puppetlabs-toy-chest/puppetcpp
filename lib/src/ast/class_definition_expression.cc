#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    class_definition_expression::class_definition_expression()
    {
    }

    class_definition_expression::class_definition_expression(lexer::position position, ast::name name, optional<vector<parameter>> parameters, optional<ast::name> parent, optional<vector<expression>> body) :
        _position(position),
        _name(rvalue_cast(name)),
        _parameters(rvalue_cast(parameters)),
        _parent(rvalue_cast(parent)),
        _body(rvalue_cast(body))
    {
    }

    ast::name const& class_definition_expression::name() const
    {
        return _name;
    }

    optional<vector<parameter>> const& class_definition_expression::parameters() const
    {
        return _parameters;
    }

    optional<ast::name> const& class_definition_expression::parent() const
    {
        return _parent;
    }

    optional<vector<expression>> const& class_definition_expression::body() const
    {
        return _body;
    }

    lexer::position const& class_definition_expression::position() const
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

