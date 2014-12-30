#include "if_expression.hpp"
#include "expression_def.hpp"
#include "utility.hpp"

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    else_expression::else_expression()
    {
    }

    else_expression::else_expression(token_position position, optional<vector<expression>> body) :
        _position(std::move(position)),
        _body(std::move(body))
    {
    }

    optional<vector<expression>> const& else_expression::body() const
    {
        return _body;
    }

    optional<vector<expression>>& else_expression::body()
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

    elsif_expression::elsif_expression(token_position position, expression conditional, optional<vector<expression>> body) :
        _position(std::move(position)),
        _conditional(std::move(conditional)),
        _body(std::move(body))
    {
    }

    expression const& elsif_expression::conditional() const
    {
        return _conditional;
    }

    expression& elsif_expression::conditional()
    {
        return _conditional;
    }

    optional<vector<expression>> const& elsif_expression::body() const
    {
        return _body;
    }

    optional<vector<expression>>& elsif_expression::body()
    {
        return _body;
    }

    token_position const& elsif_expression::position() const
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

    if_expression::if_expression(token_position position, expression conditional, optional<vector<expression>> body, optional<vector<elsif_expression>> elsifs, optional<else_expression> else_) :
        _position(std::move(position)),
        _conditional(std::move(conditional)),
        _body(std::move(body)),
        _elsifs(std::move(elsifs)),
        _else(std::move(else_))
    {
    }

    expression const& if_expression::conditional() const
    {
        return _conditional;
    }

    expression& if_expression::conditional()
    {
        return _conditional;
    }

    optional<vector<expression>> const& if_expression::body() const
    {
        return _body;
    }

    optional<vector<expression>>& if_expression::body()
    {
        return _body;
    }

    optional<vector<elsif_expression>> const& if_expression::elsifs() const
    {
        return _elsifs;
    }

    optional<vector<elsif_expression>>& if_expression::elsifs()
    {
        return _elsifs;
    }

    optional<else_expression> const& if_expression::else_() const
    {
        return _else;
    }

    optional<else_expression>& if_expression::else_()
    {
        return _else;
    }

    token_position const& if_expression::position() const
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
