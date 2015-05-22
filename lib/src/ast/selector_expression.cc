#include <puppet/ast/selector_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    selector_case_expression::selector_case_expression()
    {
    }

    selector_case_expression::selector_case_expression(expression selector, expression result) :
        selector(rvalue_cast(selector)),
        result(rvalue_cast(result))
    {
    }

    lexer::position const& selector_case_expression::position() const
    {
        return selector.position();
    }

    ostream& operator<<(ostream& os, selector_case_expression const& expr)
    {
        os << expr.selector << " => " << expr.result;
        return os;
    }

    selector_expression::selector_expression()
    {
    }

    selector_expression::selector_expression(lexer::position position, vector<selector_case_expression> cases) :
        position(rvalue_cast(position)),
        cases(rvalue_cast(cases))
    {
    }

    ostream& operator<<(ostream& os, selector_expression const& expr)
    {
        os << " ? { ";
        pretty_print(os, expr.cases, ", ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast
