#include <puppet/ast/bare_word.hpp>

using namespace std;

namespace puppet { namespace ast {

    bare_word::bare_word()
    {
    }

    ostream& operator<<(ostream& os, bare_word const& word)
    {
        os << word.value;
        return os;
    }

}}  // namespace puppet::ast
