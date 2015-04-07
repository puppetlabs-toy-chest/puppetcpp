#include <puppet/ast/parameter.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    parameter_type::parameter_type()
    {
    }

    parameter_type::parameter_type(struct type type, optional<vector<expression>> expressions) :
        _type(std::move(type)),
        _expressions(std::move(expressions))
    {
    }

    type const& parameter_type::type() const
    {
        return _type;
    }

    type& parameter_type::type()
    {
        return _type;
    }

    optional<vector<expression>> const& parameter_type::expressions() const
    {
        return _expressions;
    }

    optional<vector<expression>>& parameter_type::expressions()
    {
        return _expressions;
    }

    token_position const& parameter_type::position() const
    {
        return _type.position();
    }

    ostream& operator<<(ostream& os, parameter_type const& type)
    {
        os << type.type();

        if (type.expressions()) {
            os << "[";
            pretty_print(os, *type.expressions(), ", ");
            os << "]";
        }
        return os;
    }

    parameter::parameter() :
        _captures(false)
    {
    }

    parameter::parameter(optional<parameter_type> type, bool captures, struct variable variable, optional<expression> default_value) :
        _type(std::move(type)),
        _captures(captures),
        _variable(std::move(variable)),
        _default_value(std::move(default_value))
    {
    }

    optional<parameter_type> const& parameter::type() const
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

    token_position const& parameter::position() const
    {
        if (_type) {
            return _type->position();
        }
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
