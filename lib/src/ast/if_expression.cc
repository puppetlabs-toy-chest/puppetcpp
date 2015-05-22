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
        position(rvalue_cast(position)),
        body(rvalue_cast(body))
    {
    }

    ostream& operator<<(ostream& os, else_expression const& expr)
    {
        os << "else { ";
        pretty_print(os, expr.body, "; ");
        os << " }";
        return os;
    }

    elsif_expression::elsif_expression()
    {
    }

    elsif_expression::elsif_expression(lexer::position position, expression conditional, optional<vector<expression>> body) :
        position(rvalue_cast(position)),
        conditional(rvalue_cast(conditional)),
        body(rvalue_cast(body))
    {
    }

    ostream& operator<<(ostream& os, elsif_expression const& expr)
    {
        if (ast::is_blank(expr.conditional)) {
            return os;
        }
        os << "elsif " << expr.conditional << " { ";
        pretty_print(os, expr.body, "; ");
        os << " }";
        return os;
    }

    if_expression::if_expression()
    {
    }

    if_expression::if_expression(lexer::position position, expression conditional, optional<vector<expression>> body, optional<vector<elsif_expression>> elsifs, optional<else_expression> else_) :
        position(rvalue_cast(position)),
        conditional(rvalue_cast(conditional)),
        body(rvalue_cast(body)),
        elsifs(rvalue_cast(elsifs)),
        else_(rvalue_cast(else_))
    {
    }

    ostream& operator<<(ostream& os, if_expression const& expr)
    {
        if (is_blank(expr.conditional)) {
            return os;
        }
        os << "if " << expr.conditional << " { ";
        pretty_print(os, expr.body, "; ");
        os << " }";
        if (expr.elsifs) {
            os << " ";
            pretty_print(os, *expr.elsifs, " ");
        }
        if (expr.else_) {
            os << " " << *expr.else_;
        }
        return os;
    }

}}  // namespace puppet::ast
