#include <puppet/ast/name.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace ast {

    name::name()
    {
    }

    name::name(lexer::position position, string value) :
        _position(rvalue_cast(position)),
        _value(rvalue_cast(value))
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

    bool operator==(name const& left, name const& right)
    {
        return left.value() == right.value();
    }

    ostream& operator<<(ostream& os, name const& name)
    {
        os << name.value();
        return os;
    }

}}  // namespace puppet::ast
