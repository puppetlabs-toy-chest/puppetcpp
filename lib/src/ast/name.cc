#include <puppet/ast/name.hpp>

using namespace std;

namespace puppet { namespace ast {

    name::name()
    {
    }

    string const& name::value() const
    {
        return _value;
    }

    lexer::position const& name::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, name const& name)
    {
        os << name.value();
        return os;
    }

}}  // namespace puppet::ast
