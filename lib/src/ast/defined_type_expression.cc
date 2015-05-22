#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    defined_type_expression::defined_type_expression()
    {
    }

    defined_type_expression::defined_type_expression(lexer::position position, ast::name name, optional<vector<parameter>> parameters, optional<vector<expression>> body) :
        position(rvalue_cast(position)),
        name(rvalue_cast(name)),
        parameters(rvalue_cast(parameters)),
        body(rvalue_cast(body))
    {
    }

    ostream& operator<<(ostream& os, defined_type_expression const& expr)
    {
        if (expr.name.value.empty()) {
            return os;
        }
        os << "define " << expr.name;
        if (expr.parameters) {
            os << " (";
            pretty_print(os, expr.parameters, ", ");
            os << ")";
        }
        os << " {";
        pretty_print(os, expr.body, "; ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast

