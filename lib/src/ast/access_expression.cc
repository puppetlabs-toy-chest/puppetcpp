#include <puppet/ast/access_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    access_expression::access_expression(lexer::position position, vector<expression> arguments) :
        position(rvalue_cast(position)),
        arguments(rvalue_cast(arguments))
    {
    }

    ostream& operator<<(ostream& os, access_expression const& expr)
    {
        os << '[';
        pretty_print(os, expr.arguments, ", ");
        os << ']';
        return os;
    }

}}  // namespace puppet::ast
