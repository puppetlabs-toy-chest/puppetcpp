#include <puppet/ast/name.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    name::name()
    {
    }

    string const& name::value() const
    {
        return _value;
    }

    string& name::value()
    {
        return _value;
    }

    token_position const& name::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, name const& name)
    {
        os << name.value();
        return os;
    }

}}  // namespace puppet::ast
