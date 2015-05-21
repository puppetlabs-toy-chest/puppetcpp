#include <puppet/ast/hash.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

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
        _position(rvalue_cast(brace_position)),
        _elements(rvalue_cast(elements))
    {
    }

    optional<vector<hash_pair>> const& hash::elements() const
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
