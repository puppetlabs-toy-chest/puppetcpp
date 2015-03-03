#include <puppet/ast/resource_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/visitors.hpp>
#include <puppet/ast/utility.hpp>

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
        _name(std::move(attribute_name)),
        _op(op),
        _value(std::move(value))
    {
    }

    name const& attribute_expression::name() const
    {
        return _name;
    }

    name& attribute_expression::name()
    {
        return _name;
    }

    attribute_operator attribute_expression::op() const
    {
        return _op;
    }

    attribute_operator& attribute_expression::op()
    {
        return _op;
    }

    expression const& attribute_expression::value() const
    {
        return _value;
    }

    expression& attribute_expression::value()
    {
        return _value;
    }

    token_position const& attribute_expression::position() const
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

    resource_body::resource_body(expression title, optional<vector<attribute_expression>> attributes) :
        _title(std::move(title)),
        _attributes(std::move(attributes))
    {
    }

    expression const& resource_body::title() const
    {
        return _title;
    }

    expression& resource_body::title()
    {
        return _title;
    }

    optional<vector<attribute_expression>> const& resource_body::attributes() const
    {
        return _attributes;
    }

    optional<vector<attribute_expression>>& resource_body::attributes()
    {
        return _attributes;
    }

    token_position const& resource_body::position() const
    {
        return _title.position();
    }

    ostream& operator<<(ostream& os, resource_body const& body)
    {
        if (body.title().blank()) {
            return os;
        }
        os << body.title() << ": { ";
        pretty_print(os, body.attributes(), ", ");
        os << " }";
        return os;
    }

    resource_expression::resource_expression() :
        _status(resource_status::none)
    {
    }

    resource_expression::resource_expression(expression type, vector<resource_body> bodies, resource_status status) :
        _type(std::move(type)),
        _bodies(std::move(bodies)),
        _status(status)
    {
    }

    expression const& resource_expression::type() const
    {
        return _type;
    }

    expression& resource_expression::type()
    {
        return _type;
    }

    vector<resource_body> resource_expression::bodies() const
    {
        return _bodies;
    }

    vector<resource_body>& resource_expression::bodies()
    {
        return _bodies;
    }

    resource_status resource_expression::status() const
    {
        return _status;
    }

    resource_status& resource_expression::status()
    {
        return _status;
    }

    token_position const& resource_expression::position() const
    {
        return _type.position();
    }

    ostream& operator<<(ostream& os, resource_expression const& expression)
    {
        if (expression.status() == resource_status::none) {
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
