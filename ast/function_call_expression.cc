#include "function_call_expression.hpp"
#include "expression_def.hpp"
#include "utility.hpp"

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    function_call_expression::function_call_expression()
    {
    }

    function_call_expression::function_call_expression(name function, optional<vector<expression>> arguments, optional<struct lambda> lambda) :
        _function(std::move(function)),
        _arguments(std::move(arguments)),
        _lambda(std::move(lambda))
    {
    }

    name const& function_call_expression::function() const
    {
        return _function;
    }

    name& function_call_expression::function()
    {
        return _function;
    }

    optional<vector<expression>> const& function_call_expression::arguments() const
    {
        return _arguments;
    }

    optional<vector<expression>>& function_call_expression::arguments()
    {
        return _arguments;
    }

    optional<lambda> const& function_call_expression::lambda() const
    {
        return _lambda;
    }

    optional<lambda>& function_call_expression::lambda()
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
