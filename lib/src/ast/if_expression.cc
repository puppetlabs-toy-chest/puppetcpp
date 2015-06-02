#include <puppet/ast/if_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    else_expression::else_expression()
    {
    }

    else_expression::else_expression(lexer::position position, optional<vector<expression>> body) :
        _position(rvalue_cast(position)),
        _body(rvalue_cast(body))
    {
    }

    optional<vector<expression>> const& else_expression::body() const
    {
        return _body;
    }

    ostream& operator<<(ostream& os, else_expression const& expr)
    {
        os << "else { ";
        pretty_print(os, expr.body(), "; ");
        os << " }";
        return os;
    }

    elsif_expression::elsif_expression()
    {
    }

    elsif_expression::elsif_expression(lexer::position position, expression conditional, optional<vector<expression>> body) :
        _position(rvalue_cast(position)),
        _conditional(rvalue_cast(conditional)),
        _body(rvalue_cast(body))
    {
    }

    expression const& elsif_expression::conditional() const
    {
        return _conditional;
    }

    optional<vector<expression>> const& elsif_expression::body() const
    {
        return _body;
    }

    lexer::position const& elsif_expression::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, elsif_expression const& expr)
    {
        if (expr.conditional().blank()) {
            return os;
        }
        os << "elsif " << expr.conditional() << " { ";
        pretty_print(os, expr.body(), "; ");
        os << " }";
        return os;
    }

    if_expression::if_expression()
    {
    }

    if_expression::if_expression(lexer::position position, expression conditional, optional<vector<expression>> body, optional<vector<elsif_expression>> elsifs, optional<else_expression> else_) :
        _position(rvalue_cast(position)),
        _conditional(rvalue_cast(conditional)),
        _body(rvalue_cast(body)),
        _elsifs(rvalue_cast(elsifs)),
        _else(rvalue_cast(else_))
    {
    }

    expression const& if_expression::conditional() const
    {
        return _conditional;
    }

    optional<vector<expression>> const& if_expression::body() const
    {
        return _body;
    }

    optional<vector<elsif_expression>> const& if_expression::elsifs() const
    {
        return _elsifs;
    }

    optional<else_expression> const& if_expression::else_() const
    {
        return _else;
    }

    lexer::position const& if_expression::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, if_expression const& expr)
    {
        if (expr.conditional().blank()) {
            return os;
        }
        os << "if " << expr.conditional() << " { ";
        pretty_print(os, expr.body(), "; ");
        os << " }";
        if (expr.elsifs()) {
            os << " ";
            pretty_print(os, *expr.elsifs(), " ");
        }
        if (expr.else_()) {
            os << " " << *expr.else_();
        }
        return os;
    }

}}  // namespace puppet::ast
