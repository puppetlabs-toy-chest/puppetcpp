#include <puppet/ast/lambda.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    lambda::lambda()
    {
    }

    lambda::lambda(lexer::position position, optional<vector<parameter>> parameters, optional<vector<expression>> body) :
        position(rvalue_cast(position)),
        parameters(rvalue_cast(parameters)),
        body(rvalue_cast(body))
    {
    }

    ostream& operator<<(ostream& os, ast::lambda const& lambda)
    {
        os << "|";
        pretty_print(os, lambda.parameters, ", ");
        os << "| { ";
        pretty_print(os, lambda.body, "; ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast
