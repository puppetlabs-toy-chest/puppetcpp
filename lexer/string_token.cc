#include "string_token.hpp"

using namespace std;

namespace puppet { namespace lexer {

    ostream& operator<<(ostream& os, string_token const& token)
    {
        os << (token.interpolated() ? '"' : '\'') << token.text() << (token.interpolated() ? '"' : '\'');
        return os;
    }

}}  // namespace puppet::lexer
