#include <puppet/ast/case_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using namespace puppet::lexer;
using boost::optional;

namespace puppet { namespace ast {

    case_proposition::case_proposition()
    {
    }

    case_proposition::case_proposition(vector<expression> options, optional<vector<expression>> body) :
        _options(std::move(options)),
        _body(std::move(body))
    {
        if (!_options.empty()) {
            _position = _options.front().position();
        }
    }

    case_proposition::case_proposition(token_position position, optional<vector<expression>> body) :
        _position(std::move(position)),
        _body(std::move(body))
    {
    }

    vector<expression> const& case_proposition::options() const
    {
        return _options;
    }

    vector<expression>& case_proposition::options()
    {
        return _options;
    }

    optional<vector<expression>> const& case_proposition::body() const
    {
        return _body;
    }

    optional<vector<expression>>& case_proposition::body()
    {
        return _body;
    }

    bool case_proposition::is_default() const
    {
        return _options.empty();
    }

    token_position const& case_proposition::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, case_proposition const& proposition)
    {
        if (proposition.is_default()) {
            os << "default";
        } else {
            pretty_print(os, proposition.options(), ", ");
        }
        os << ": {";
        pretty_print(os, proposition.body(), "; ");
        os << "}";
        return os;
    }

    case_expression::case_expression()
    {
    }

    case_expression::case_expression(lexer::token_position position, struct expression expression, vector<case_proposition> propositions) :
        _position(std::move(position)),
        _expression(std::move(expression)),
        _propositions(std::move(propositions))
    {
    }

    expression const& case_expression::expression() const
    {
        return _expression;
    }

    expression& case_expression::expression()
    {
        return _expression;
    }

    vector<case_proposition> const& case_expression::propositions() const
    {
        return _propositions;
    }

    vector<case_proposition>& case_expression::propositions()
    {
        return _propositions;
    }

    token_position const& case_expression::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, case_expression const& expr)
    {
        if (expr.expression().blank()) {
            return os;
        }
        os << "case " << expr.expression() << " { ";
        pretty_print(os, expr.propositions(), " ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast
