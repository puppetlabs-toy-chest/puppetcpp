#include <puppet/ast/case_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    case_proposition::case_proposition()
    {
    }

    case_proposition::case_proposition(vector<expression> options, optional<vector<expression>> body) :
        options(rvalue_cast(options)),
        body(rvalue_cast(body))
    {
        if (!options.empty()) {
            position = options.front().position();
        }
    }

    case_proposition::case_proposition(ast::lambda option, optional<vector<expression>> body) :
        lambda(rvalue_cast(option)),
        body(rvalue_cast(body))
    {
        position = lambda->position;
    }

    ostream& operator<<(ostream& os, case_proposition const& proposition)
    {
        pretty_print(os, proposition.options, ", ");
        os << ": {";
        pretty_print(os, proposition.body, "; ");
        os << "}";
        return os;
    }

    case_expression::case_expression()
    {
    }

    case_expression::case_expression(lexer::position position, struct expression expression, vector<case_proposition> propositions) :
        position(rvalue_cast(position)),
        expression(rvalue_cast(expression)),
        propositions(rvalue_cast(propositions))
    {
    }

    ostream& operator<<(ostream& os, case_expression const& expr)
    {
        if (is_blank(expr.expression)) {
            return os;
        }
        os << "case " << expr.expression << " { ";
        pretty_print(os, expr.propositions, " ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast
