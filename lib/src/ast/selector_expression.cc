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
        _position(selector.position()),
        _selector(std::move(selector)),
        _result(std::move(result))
    {
    }

    selector_case_expression::selector_case_expression(token_position position, expression result) :
        _position(std::move(position)),
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

    bool selector_case_expression::is_default() const
    {
        return _selector.blank();
    }

    token_position const& selector_case_expression::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, selector_case_expression const& expr)
    {
        if (expr.is_default()) {
            os << "default";
        } else {
            os << expr.selector();
        }
        os << " => " << expr.result();
        return os;
    }

    selector_expression::selector_expression()
    {
    }

    selector_expression::selector_expression(primary_expression value, vector<selector_case_expression> cases) :
        _value(std::move(value)),
        _cases(std::move(cases))
    {
    }

    primary_expression const& selector_expression::value() const
    {
        return _value;
    }

    vector<selector_case_expression> const& selector_expression::cases() const
    {
        return _cases;
    }

    token_position const& selector_expression::position() const
    {
        return get_position(_value);
    }

    ostream& operator<<(ostream& os, selector_expression const& expr)
    {
        os << expr.value() << " ? { ";
        pretty_print(os, expr.cases(), ", ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast
