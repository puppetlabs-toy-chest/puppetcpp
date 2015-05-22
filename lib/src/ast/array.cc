#include <puppet/ast/array.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using boost::optional;

namespace puppet { namespace ast {

    array::array(lexer::position position, optional<vector<expression>> elements) :
        position(rvalue_cast(position)),
        elements(rvalue_cast(elements))
    {
    }

    ostream& operator<<(ostream& os, array const& array)
    {
        os << '[';
        pretty_print(os, array.elements, ", ");
        os << ']';
        return os;
    }

}}  // namespace puppet::ast
