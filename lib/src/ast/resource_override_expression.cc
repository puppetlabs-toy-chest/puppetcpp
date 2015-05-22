#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    resource_override_expression::resource_override_expression()
    {
    }

    resource_override_expression::resource_override_expression(primary_expression reference, optional<vector<attribute_expression>> attributes) :
        reference(rvalue_cast(reference)),
        attributes(rvalue_cast(attributes))
    {
    }

    ostream& operator<<(ostream& os, resource_override_expression const& expr)
    {
        if (is_blank(expr.reference)) {
            return os;
        }
        os << expr.reference << " { ";
        pretty_print(os, expr.attributes, ", ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast

