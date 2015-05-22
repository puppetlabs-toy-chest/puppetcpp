#include <puppet/ast/method_call_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    method_call_expression::method_call_expression()
    {
    }

    method_call_expression::method_call_expression(name method, optional<vector<expression>> arguments, optional<ast::lambda> lambda) :
        method(rvalue_cast(method)),
        arguments(rvalue_cast(arguments)),
        lambda(rvalue_cast(lambda))
    {
    }

    lexer::position const& method_call_expression::position() const
    {
        return method.position;
    }

    ostream& operator<<(ostream& os, method_call_expression const& expr)
    {
        if (expr.method.value.empty()) {
            return os;
        }
        os << "." << expr.method << "(";
        pretty_print(os, expr.arguments, ", ");
        os << ")";
        if (expr.lambda) {
            os << " " << *expr.lambda;
        }
        return os;
    }

}}  // namespace puppet::ast
