#include <puppet/ast/method_call_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    method_call::method_call()
    {
    }

    method_call::method_call(name method, optional<vector<expression>> arguments, optional<struct lambda> lambda) :
        _method(std::move(method)),
        _arguments(std::move(arguments)),
        _lambda(std::move(lambda))
    {
    }

    name const& method_call::method() const
    {
        return _method;
    }

    name& method_call::method()
    {
        return _method;
    }

    optional<vector<expression>> const& method_call::arguments() const
    {
        return _arguments;
    }

    optional<vector<expression>>& method_call::arguments()
    {
        return _arguments;
    }

    optional<lambda> const& method_call::lambda() const
    {
        return _lambda;
    }

    optional<lambda>& method_call::lambda()
    {
        return _lambda;
    }

    token_position const& method_call::position() const
    {
        return _method.position();
    }

    ostream& operator<<(ostream& os, method_call const& call)
    {
        if (call.method().value().empty()) {
            return os;
        }
        os << "." << call.method() << "(";
        pretty_print(os, call.arguments(), ", ");
        os << ")";
        if (call.lambda()) {
            os << " " << *call.lambda();
        }
        return os;
    }

    method_call_expression::method_call_expression()
    {
    }

    method_call_expression::method_call_expression(primary_expression target, vector<method_call> calls) :
        _target(std::move(target)),
        _calls(std::move(calls))
    {
    }

    primary_expression const& method_call_expression::target() const
    {
        return _target;
    }

    primary_expression& method_call_expression::target()
    {
        return _target;
    }

    vector<method_call> const& method_call_expression::calls() const
    {
        return _calls;
    }

    vector<method_call>& method_call_expression::calls()
    {
        return _calls;
    }

    token_position const& method_call_expression::position() const
    {
        return get_position(_target);
    }

    ostream& operator<<(ostream& os, method_call_expression const& expr)
    {
        if (is_blank(expr.target()) || expr.calls().empty()) {
            return os;
        }
        os << expr.target();
        for (auto const& call : expr.calls()) {
            os << call;
        }
        return os;
    }

}}  // namespace puppet::ast
