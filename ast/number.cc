#include "number.hpp"

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    number::number()
    {
    }

    string const& number::value() const
    {
        return _value;
    }

    string& number::value()
    {
        return _value;
    }

    token_position const& number::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, number const& number)
    {
        os << number.value();
        return os;
    }

}}  // namespace puppet::ast
