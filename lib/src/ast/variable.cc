#include <puppet/ast/variable.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    variable::variable()
    {
    }

    variable::variable(lexer::position position, string name) :
        _position(rvalue_cast(position)),
        _name(rvalue_cast(name))
    {

    }

    string const& variable::name() const
    {
        return _name;
    }

    lexer::position const& variable::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, variable const& var)
    {
        os << "$" << var.name();
        return os;
    }

}}  // namespace puppet::ast
