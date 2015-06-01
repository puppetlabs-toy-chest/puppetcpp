#include <puppet/ast/unless_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    unless_expression::unless_expression()
    {
    }

    unless_expression::unless_expression(lexer::position position, expression conditional, optional<vector<expression>> body, optional<else_expression> else_) :
        _position(rvalue_cast(position)),
        _conditional(rvalue_cast(conditional)),
        _body(rvalue_cast(body)),
        _else(rvalue_cast(else_))
    {
    }

    expression const& unless_expression::conditional() const
    {
        return _conditional;
    }

    optional<vector<expression>> const& unless_expression::body() const
    {
        return _body;
    }

    optional<else_expression> const& unless_expression::else_() const
    {
        return _else;
    }

    lexer::position const& unless_expression::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, unless_expression const& expr)
    {
        if (expr.conditional().blank()) {
            return os;
        }
        os << "unless " << expr.conditional() << " { ";
        pretty_print(os, expr.body(), "; ");
        os << " }";
        if (expr.else_()) {
            os << " " << *expr.else_();
        }
        return os;
    }

}}  // namespace puppet::ast
