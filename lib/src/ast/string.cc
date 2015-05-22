#include <puppet/ast/string.hpp>

using namespace std;

namespace puppet { namespace ast {

    string::string() :
        quote(0),
        interpolated(false),
        margin(0),
        remove_break(false)
    {
    }

    ostream& operator<<(ostream& os, string const& str)
    {
        os << (str.interpolated ? '"' : '\'') << str.value << (str.interpolated ? '"' : '\'');
        return os;
    }

}}  // namespace puppet::ast
