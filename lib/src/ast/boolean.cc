#include <puppet/ast/boolean.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    boolean::boolean() :
        _value(false)
    {
    }

    boolean::boolean(token_position position, bool value) :
        _position(std::move(position)),
        _value(value)
    {
    }

    bool boolean::value() const
    {
        return _value;
    }

    token_position const& boolean::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, boolean const& b)
    {
        os << (b.value() ? "true" : "false");
        return os;
    }

}}  // namespace puppet::ast
