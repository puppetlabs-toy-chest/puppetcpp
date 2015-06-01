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
        _selector(rvalue_cast(selector)),
        _result(rvalue_cast(result))
    {
    }

    expression const& selector_case_expression::selector() const
    {
        return _selector;
    }

    expression const& selector_case_expression::result() const
    {
        return _result;
    }

    lexer::position const& selector_case_expression::position() const
    {
        return _selector.position();
    }

    ostream& operator<<(ostream& os, selector_case_expression const& expr)
    {
        os << expr.selector() << " => " << expr.result();
        return os;
    }

    selector_expression::selector_expression()
    {
    }

    selector_expression::selector_expression(lexer::position position, vector<selector_case_expression> cases) :
        _position(rvalue_cast(position)),
        _cases(rvalue_cast(cases))
    {
    }

    vector<selector_case_expression> const& selector_expression::cases() const
    {
        return _cases;
    }

    lexer::position const& selector_expression::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, selector_expression const& expr)
    {
        os << " ? { ";
        pretty_print(os, expr.cases(), ", ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast
