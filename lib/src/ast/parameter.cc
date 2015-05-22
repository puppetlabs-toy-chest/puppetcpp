#include <puppet/ast/parameter.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using boost::optional;

namespace puppet { namespace ast {

    parameter::parameter() :
        captures(false)
    {
    }

    parameter::parameter(optional<primary_expression> type, bool captures, ast::variable variable, optional<expression> default_value) :
        type(rvalue_cast(type)),
        captures(captures),
        variable(rvalue_cast(variable)),
        default_value(rvalue_cast(default_value))
    {
    }

    lexer::position const& parameter::position() const
    {
        if (type) {
            return get_position(*type);
        }
        return variable.position;
    }

    ostream& operator<<(ostream& os, parameter const& param)
    {
        if (param.type) {
            os << *param.type << " ";
        }
        if (param.captures) {
            os << '*';
        }
        os << param.variable;
        if (param.default_value) {
            os << " = " << *param.default_value;
        }
        return os;
    }

}}  // namespace puppet::ast
