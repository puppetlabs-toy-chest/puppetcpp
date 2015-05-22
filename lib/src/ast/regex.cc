#include <puppet/ast/regex.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    regex::regex()
    {
    }

    ostream& operator<<(ostream& os, regex const& regex)
    {
        os << "/" << regex.value << "/";
        return os;
    }

}}  // namespace puppet::ast
