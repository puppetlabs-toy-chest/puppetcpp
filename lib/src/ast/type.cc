#include <puppet/ast/type.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    type::type()
    {
    }

    string const& type::name() const
    {
        return _name;
    }

    token_position const& type::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, ast::type const& type)
    {
        os << type.name();
        return os;
    }

}}  // namespace puppet::ast
