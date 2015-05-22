#include <puppet/ast/undef.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    undef::undef(lexer::position position) :
        position(rvalue_cast(position))
    {
    }

    ostream& operator<<(ostream& os, undef const&)
    {
        os << "undef";
        return os;
    }

}}  // namespace puppet::ast
