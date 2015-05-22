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
        position(rvalue_cast(position)),
        conditional(rvalue_cast(conditional)),
        body(rvalue_cast(body)),
        else_(rvalue_cast(else_))
    {
    }

    ostream& operator<<(ostream& os, unless_expression const& expr)
    {
        if (is_blank(expr.conditional)) {
            return os;
        }
        os << "unless " << expr.conditional << " { ";
        pretty_print(os, expr.body, "; ");
        os << " }";
        if (expr.else_) {
            os << " " << *expr.else_;
        }
        return os;
    }

}}  // namespace puppet::ast
