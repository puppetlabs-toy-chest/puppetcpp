#include <puppet/ast/type.hpp>

using namespace std;

namespace puppet { namespace ast {

    type::type()
    {
    }

    ostream& operator<<(ostream& os, ast::type const& type)
    {
        os << type.name;
        return os;
    }

}}  // namespace puppet::ast
