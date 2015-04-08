#include <puppet/ast/bare_word.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    bare_word::bare_word()
    {
    }

    string const& bare_word::value() const
    {
        return _value;
    }

    token_position const& bare_word::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, bare_word const& word)
    {
        os << word.value();
        return os;
    }

}}  // namespace puppet::ast
