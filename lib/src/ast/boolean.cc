#include <puppet/ast/boolean.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace ast {

    boolean::boolean() :
        _value(false)
    {
    }

    boolean::boolean(lexer::position position, bool value) :
        _position(rvalue_cast(position)),
        _value(value)
    {
    }

    bool boolean::value() const
    {
        return _value;
    }

    lexer::position const& boolean::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, boolean const& b)
    {
        os << (b.value() ? "true" : "false");
        return os;
    }

}}  // namespace puppet::ast
