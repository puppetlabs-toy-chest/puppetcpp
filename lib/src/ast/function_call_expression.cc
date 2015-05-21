#include <puppet/ast/function_call_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    function_call_expression::function_call_expression()
    {
    }

    function_call_expression::function_call_expression(name function, optional<vector<expression>> arguments, optional<struct lambda> lambda) :
        _function(rvalue_cast(function)),
        _arguments(rvalue_cast(arguments)),
        _lambda(rvalue_cast(lambda))
    {
    }

    name const& function_call_expression::function() const
    {
        return _function;
    }

    optional<vector<expression>> const& function_call_expression::arguments() const
    {
        return _arguments;
    }

    optional<lambda> const& function_call_expression::lambda() const
    {
        return _lambda;
    }

    token_position const& function_call_expression::position() const
    {
        return _function.position();
    }

    ostream& operator<<(ostream& os, function_call_expression const& expr)
    {
        if (expr.function().value().empty()) {
            return os;
        }
        os << expr.function() << "(";
        pretty_print(os, expr.arguments(), ", ");
        os << ")";
        if (expr.lambda()) {
            os << " " << *expr.lambda();
        }
        return os;
    }

}}  // namespace puppet::ast
