#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    class_definition_expression::class_definition_expression()
    {
    }

    class_definition_expression::class_definition_expression(lexer::position position, ast::name name, optional<vector<parameter>> parameters, optional<ast::name> parent, optional<vector<expression>> body) :
        position(position),
        name(rvalue_cast(name)),
        parameters(rvalue_cast(parameters)),
        parent(rvalue_cast(parent)),
        body(rvalue_cast(body))
    {
    }

    ostream& operator<<(ostream& os, class_definition_expression const& expr)
    {
        if (expr.name.value.empty()) {
            return os;
        }

        os << "class " << expr.name;
        if (expr.parameters) {
            os << " (";
            pretty_print(os, expr.parameters, ", ");
            os << ")";
        }
        if (expr.parent) {
            os << " inherits " << *expr.parent;
        }
        os << " {";
        pretty_print(os, expr.body, "; ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast

