#include <puppet/ast/bare_word.hpp>

using namespace std;

namespace puppet { namespace ast {

    string const& bare_word::value() const
    {
        return _value;
    }

    lexer::position const& bare_word::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, bare_word const& word)
    {
        os << word.value();
        return os;
    }

}}  // namespace puppet::ast
