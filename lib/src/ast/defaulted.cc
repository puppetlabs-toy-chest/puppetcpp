#include <puppet/ast/defaulted.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    defaulted::defaulted(lexer::position position) :
        position(rvalue_cast(position))
    {
    }

    ostream& operator<<(ostream& os, defaulted const&)
    {
        os << "default";
        return os;
    }

}}  // namespace puppet::ast
