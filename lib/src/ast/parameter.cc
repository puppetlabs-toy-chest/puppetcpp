#include <puppet/ast/parameter.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using boost::optional;

namespace puppet { namespace ast {

    parameter::parameter() :
        _captures(false)
    {
    }

    parameter::parameter(optional<primary_expression> type, bool captures, ast::variable variable, optional<expression> default_value) :
        _type(rvalue_cast(type)),
        _captures(captures),
        _variable(rvalue_cast(variable)),
        _default_value(rvalue_cast(default_value))
    {
    }

    optional<primary_expression> const& parameter::type() const
    {
        return _type;
    }

    bool parameter::captures() const
    {
        return _captures;
    }

    variable const& parameter::variable() const
    {
        return _variable;
    }

    optional<expression> const& parameter::default_value() const
    {
        return _default_value;
    }

    lexer::position const& parameter::position() const
    {
        return _variable.position();
    }

    ostream& operator<<(ostream& os, parameter const& param)
    {
        if (param.type()) {
            os << *param.type() << " ";
        }
        if (param.captures()) {
            os << '*';
        }
        os << param.variable();
        if (param.default_value()) {
            os << " = " << *param.default_value();
        }
        return os;
    }

}}  // namespace puppet::ast
