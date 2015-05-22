#include <puppet/ast/name.hpp>

using namespace std;

namespace puppet { namespace ast {

    name::name()
    {
    }

    ostream& operator<<(ostream& os, name const& name)
    {
        os << name.value;
        return os;
    }

}}  // namespace puppet::ast
