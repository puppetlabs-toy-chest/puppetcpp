#include <puppet/ast/resource_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    ostream& operator<<(ostream& os, attribute_operator op)
    {
        switch (op) {
            case attribute_operator::none:
                break;

            case attribute_operator::assignment:
                os << "=>";
                break;

            case attribute_operator::append:
                os << "+>";
                break;

            default:
                throw runtime_error("invalid attribute operator.");
        }
        return os;
    }

    attribute_expression::attribute_expression() :
        _op(attribute_operator::none)
    {
    }

    attribute_expression::attribute_expression(struct name attribute_name, attribute_operator op, expression value) :
        _name(rvalue_cast(attribute_name)),
        _op(op),
        _value(rvalue_cast(value))
    {
    }

    name const& attribute_expression::name() const
    {
        return _name;
    }

    attribute_operator attribute_expression::op() const
    {
        return _op;
    }

    expression const& attribute_expression::value() const
    {
        return _value;
    }

    lexer::position const& attribute_expression::position() const
    {
        return _name.position();
    }

    ostream& operator<<(ostream& os, attribute_expression const& attribute)
    {
        if (attribute.op() == attribute_operator::none) {
            return os;
        }
        os << attribute.name() << " " << attribute.op() << " " << attribute.value();
        return os;
    }

    resource_body::resource_body()
    {
    }

    resource_body::resource_body(primary_expression title, optional<vector<attribute_expression>> attributes) :
        _title(rvalue_cast(title)),
        _attributes(rvalue_cast(attributes))
    {
    }

    primary_expression const& resource_body::title() const
    {
        return _title;
    }

    optional<vector<attribute_expression>> const& resource_body::attributes() const
    {
        return _attributes;
    }

    lexer::position const& resource_body::position() const
    {
        return get_position(_title);
    }

    ostream& operator<<(ostream& os, resource_body const& body)
    {
        os << body.title() << ": ";
        pretty_print(os, body.attributes(), ", ");
        return os;
    }

    resource_expression::resource_expression() :
        _status(resource_status::realized)
    {
    }

    resource_expression::resource_expression(ast::primary_expression type, vector<resource_body> bodies, resource_status status) :
        _type(rvalue_cast(type)),
        _bodies(rvalue_cast(bodies)),
        _status(status)
    {
    }

    ast::primary_expression const& resource_expression::type() const
    {
        return _type;
    }

    vector<resource_body> const& resource_expression::bodies() const
    {
        return _bodies;
    }

    resource_status resource_expression::status() const
    {
        return _status;
    }

    lexer::position const& resource_expression::position() const
    {
        return get_position(_type);
    }

    ostream& operator<<(ostream& os, resource_expression const& expression)
    {
        if (is_blank(expression.type())) {
            return os;
        }
        if (expression.status() == resource_status::virtualized) {
            os << "@";
        } else if (expression.status() == resource_status::exported) {
            os << "@@";
        }
        os << expression.type() << " { ";
        pretty_print(os, expression.bodies(), "; ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast
