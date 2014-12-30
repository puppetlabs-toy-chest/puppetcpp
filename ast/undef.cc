#include "undef.hpp"

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    undef::undef()
    {
    }

    undef::undef(token_position position) :
        _position(std::move(position))
    {
    }

    token_position const& undef::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, undef const&)
    {
        os << "undef";
        return os;
    }

}}  // namespace puppet::ast
