#include <puppet/ast/postfix_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    postfix_expression::postfix_expression()
    {
    }

    postfix_expression::postfix_expression(primary_expression primary, vector<postfix_subexpression> subexpressions) :
        primary(rvalue_cast(primary)),
        subexpressions(rvalue_cast(subexpressions))
    {
    }

    lexer::position const& postfix_expression::position() const
    {
        return get_position(primary);
    }

    ostream& operator<<(ostream& os, postfix_expression const& expr)
    {
        os << expr.primary;
        for (auto const& subexpression : expr.subexpressions) {
            os << subexpression;
        }
        return os;
    }

}}  // namespace puppet::ast
