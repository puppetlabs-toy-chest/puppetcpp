#include <puppet/ast/variable.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    variable::variable()
    {
    }

    variable::variable(lexer::position position, string name) :
        position(rvalue_cast(position)),
        name(rvalue_cast(name))
    {
    }

    ostream& operator<<(ostream& os, variable const& var)
    {
        os << "$" << var.name;
        return os;
    }

}}  // namespace puppet::ast
