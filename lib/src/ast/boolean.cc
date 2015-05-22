#include <puppet/ast/boolean.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace ast {

    boolean::boolean() :
        value(false)
    {
    }

    boolean::boolean(lexer::position position, bool value) :
        position(rvalue_cast(position)),
        value(value)
    {
    }

    ostream& operator<<(ostream& os, boolean const& b)
    {
        os << (b.value ? "true" : "false");
        return os;
    }

}}  // namespace puppet::ast
