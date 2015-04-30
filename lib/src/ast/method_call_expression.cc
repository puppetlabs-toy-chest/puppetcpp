#include <puppet/ast/method_call_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    method_call_expression::method_call_expression()
    {
    }

    method_call_expression::method_call_expression(name method, optional<vector<expression>> arguments, optional<struct lambda> lambda) :
        _method(std::move(method)),
        _arguments(std::move(arguments)),
        _lambda(std::move(lambda))
    {
    }

    name const& method_call_expression::method() const
    {
        return _method;
    }

    optional<vector<expression>> const& method_call_expression::arguments() const
    {
        return _arguments;
    }

    optional<lambda> const& method_call_expression::lambda() const
    {
        return _lambda;
    }

    token_position const& method_call_expression::position() const
    {
        return _method.position();
    }

    ostream& operator<<(ostream& os, method_call_expression const& expr)
    {
        if (expr.method().value().empty()) {
            return os;
        }
        os << "." << expr.method() << "(";
        pretty_print(os, expr.arguments(), ", ");
        os << ")";
        if (expr.lambda()) {
            os << " " << *expr.lambda();
        }
        return os;
    }

}}  // namespace puppet::ast
