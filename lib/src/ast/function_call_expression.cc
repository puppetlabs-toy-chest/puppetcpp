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

    function_call_expression::function_call_expression(name function, optional<vector<expression>> arguments, optional<ast::lambda> lambda) :
        function(rvalue_cast(function)),
        arguments(rvalue_cast(arguments)),
        lambda(rvalue_cast(lambda))
    {
    }

    lexer::position const& function_call_expression::position() const
    {
        return function.position;
    }

    ostream& operator<<(ostream& os, function_call_expression const& expr)
    {
        if (expr.function.value.empty()) {
            return os;
        }
        os << expr.function << "(";
        pretty_print(os, expr.arguments, ", ");
        os << ")";
        if (expr.lambda) {
            os << " " << *expr.lambda;
        }
        return os;
    }

}}  // namespace puppet::ast
