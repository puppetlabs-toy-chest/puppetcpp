#include <puppet/ast/selector_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    selector_case_expression::selector_case_expression()
    {
    }

    selector_case_expression::selector_case_expression(expression selector, expression result) :
        _selector(std::move(selector)),
        _result(std::move(result))
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

    token_position const& selector_case_expression::position() const
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

    selector_expression::selector_expression(token_position position, vector<selector_case_expression> cases) :
        _position(std::move(position)),
        _cases(std::move(cases))
    {
    }

    vector<selector_case_expression> const& selector_expression::cases() const
    {
        return _cases;
    }

    token_position const& selector_expression::position() const
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
