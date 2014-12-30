#include "expression_def.hpp"
#include "utility.hpp"

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    resource_override_expression::resource_override_expression()
    {
    }

    resource_override_expression::resource_override_expression(expression reference, optional<vector<attribute_expression>> attributes) :
        _reference(std::move(reference)),
        _attributes(std::move(attributes))
    {
    }

    expression const& resource_override_expression::reference() const
    {
        return _reference;
    }

    expression& resource_override_expression::reference()
    {
        return _reference;
    }

    optional<vector<attribute_expression>> const& resource_override_expression::attributes() const
    {
        return _attributes;
    }

    optional<vector<attribute_expression>>& resource_override_expression::attributes()
    {
        return _attributes;
    }

    token_position const& resource_override_expression::position() const
    {
        return _reference.position();
    }

    ostream& operator<<(ostream& os, resource_override_expression const& expr)
    {
        if (expr.reference().blank()) {
            return os;
        }
        os << expr.reference() << " { ";
        pretty_print(os, expr.attributes(), ", ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast

