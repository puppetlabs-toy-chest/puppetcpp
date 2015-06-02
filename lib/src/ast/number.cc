#include <puppet/ast/number.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    number::number()
    {
    }

    number::number(number_token const& token) :
        _position(token.position()),
        _value(token.value())
    {
    }

    number::value_type const& number::value() const
    {
        return _value;
    }

    lexer::position const& number::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, number const& number)
    {
        os << number.value();
        return os;
    }

}}  // namespace puppet::ast
