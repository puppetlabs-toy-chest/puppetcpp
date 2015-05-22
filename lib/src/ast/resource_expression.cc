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
        op(attribute_operator::none)
    {
    }

    attribute_expression::attribute_expression(struct name attribute_name, attribute_operator op, expression value) :
        name(rvalue_cast(attribute_name)),
        op(op),
        value(rvalue_cast(value))
    {
    }

    lexer::position const& attribute_expression::position() const
    {
        return name.position;
    }

    ostream& operator<<(ostream& os, attribute_expression const& attribute)
    {
        if (attribute.op == attribute_operator::none) {
            return os;
        }
        os << attribute.name << " " << attribute.op << " " << attribute.value;
        return os;
    }

    resource_body::resource_body()
    {
    }

    resource_body::resource_body(expression title, optional<vector<attribute_expression>> attributes) :
        title(rvalue_cast(title)),
        attributes(rvalue_cast(attributes))
    {
    }

    lexer::position const& resource_body::position() const
    {
        return title.position();
    }

    ostream& operator<<(ostream& os, resource_body const& body)
    {
        if (is_blank(body.title)) {
            return os;
        }
        os << body.title << ": { ";
        pretty_print(os, body.attributes, ", ");
        os << " }";
        return os;
    }

    resource_expression::resource_expression() :
        status(resource_status::realized)
    {
    }

    resource_expression::resource_expression(ast::name type, vector<resource_body> bodies, resource_status status) :
        type(rvalue_cast(type)),
        bodies(rvalue_cast(bodies)),
        status(status)
    {
    }

    lexer::position const& resource_expression::position() const
    {
        return type.position;
    }

    ostream& operator<<(ostream& os, resource_expression const& expression)
    {
        if (expression.type.value.empty()) {
            return os;
        }
        if (expression.status == resource_status::virtualized) {
            os << "@";
        } else if (expression.status == resource_status::exported) {
            os << "@@";
        }
        os << expression.type << " { ";
        pretty_print(os, expression.bodies, "; ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast
