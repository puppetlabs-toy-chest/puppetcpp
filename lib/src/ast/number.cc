#include <puppet/ast/number.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    number::number()
    {
    }

    number::number(number_token const& token) :
        position(token.position()),
        value(token.value())
    {
    }

    ostream& operator<<(ostream& os, number const& number)
    {
        os << number.value;
        return os;
    }

}}  // namespace puppet::ast
