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
//
//    resource_reference::resource_reference()
//    {
//    }
//
//    resource_reference::resource_reference(struct type type, vector<expression> names) :
//        _type(std::move(type)),
//        _names(std::move(names))
//    {
//    }
//
//    type const& resource_reference::type() const
//    {
//        return _type;
//    }
//
//    type & resource_reference::type()
//    {
//        return _type;
//    }
//
//    vector<expression> const& resource_reference::names() const
//    {
//        return _names;
//    }
//
//    vector<expression>& resource_reference::names()
//    {
//        return _names;
//    }
//
//    token_position const& resource_reference::position() const
//    {
//        return _type.position();
//    }
//
//    ostream& operator<<(ostream& os, resource_reference const& reference)
//    {
//        if (reference.type().name().empty()) {
//            return os;
//        }
//        os << reference.type() << "[";
//        pretty_print(os, reference.names(), ", ");
//        os << "]";
//        return os;
//    }
//
//    resource_override::resource_override()
//    {
//    }
//
//    resource_override::resource_override(resource_reference reference, optional<vector<attribute_expression>> attributes) :
//        resource_reference(std::move(reference)),
//        _attributes(std::move(attributes))
//    {
//    }
//
//    optional<vector<attribute_expression>> const& resource_override::attributes() const
//    {
//        return _attributes;
//    }
//
//    optional<vector<attribute_expression>>& resource_override::attributes()
//    {
//        return _attributes;
//    }
//
//    ostream& operator<<(ostream& os, resource_override const& override)
//    {
//        if (override.type().name().empty()) {
//            return os;
//        }
//        os << override.type() << "[";
//        pretty_print(os, override.names(), ", ");
//        os << "] { ";
//        pretty_print(os, override.attributes(), ", ");
//        os << " }";
//        return os;
//    }
//
//    ostream& operator<<(ostream& os, resource const& res)
//    {
//        return boost::apply_visitor(insertion_visitor(os), res);
//    }

}}  // namespace puppet::ast
