#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    resource_defaults_expression::resource_defaults_expression()
    {
    }

    resource_defaults_expression::resource_defaults_expression(ast::type type, optional<vector<attribute_expression>> attributes) :
        type(rvalue_cast(type)),
        attributes(rvalue_cast(attributes))
    {
    }

    lexer::position const& resource_defaults_expression::position() const
    {
        return type.position;
    }

    ostream& operator<<(ostream& os, resource_defaults_expression const& expr)
    {
        if (expr.type.name.empty()) {
            return os;
        }
        os << expr.type << " { ";
        pretty_print(os, expr.attributes, ", ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast

