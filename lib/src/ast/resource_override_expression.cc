#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    resource_override_expression::resource_override_expression()
    {
    }

    resource_override_expression::resource_override_expression(primary_expression reference, optional<vector<attribute_expression>> attributes) :
        _reference(std::move(reference)),
        _attributes(std::move(attributes))
    {
    }

    primary_expression const& resource_override_expression::reference() const
    {
        return _reference;
    }

    optional<vector<attribute_expression>> const& resource_override_expression::attributes() const
    {
        return _attributes;
    }

    token_position const& resource_override_expression::position() const
    {
        return get_position(_reference);
    }

    ostream& operator<<(ostream& os, resource_override_expression const& expr)
    {
        if (is_blank(expr.reference())) {
            return os;
        }
        os << expr.reference() << " { ";
        pretty_print(os, expr.attributes(), ", ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast

