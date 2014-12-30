#include "hash.hpp"
#include "expression_def.hpp"
#include "utility.hpp"

using namespace std;
using namespace boost;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    ostream& operator<<(ostream& os, hash_pair const& pair)
    {
        os << pair.first << " => " << pair.second;
        return os;
    }

    hash::hash()
    {
    }

    hash::hash(token_position brace_position, optional<vector<pair<expression, expression>>> elements) :
        _position(std::move(brace_position)),
        _elements(std::move(elements))
    {
    }

    optional<vector<hash_pair>> const& hash::elements() const
    {
        return _elements;
    }

    optional<vector<hash_pair>>& hash::elements()
    {
        return _elements;
    }

    lexer::token_position const& hash::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, hash const& hash)
    {
        os << "{";
        pretty_print(os, hash.elements(), ", ");
        os << "}";
        return os;
    }

}}  // namespace puppet::ast
