#include <puppet/ast/array.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using namespace boost;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    array::array()
    {
    }

    array::array(token_position bracket_position, optional<vector<expression>> elements) :
        _position(std::move(bracket_position)),
        _elements(std::move(elements))
    {
    }

    optional<vector<expression>> const&array::elements() const
    {
        return _elements;
    }

    optional<vector<expression>>& array::elements()
    {
        return _elements;
    }

    token_position const& array::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, array const& array)
    {
        os << '[';
        pretty_print(os, array.elements(), ", ");
        os << ']';
        return os;
    }

}}  // namespace puppet::ast
