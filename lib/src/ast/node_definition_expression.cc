#include <puppet/ast/node_definition_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>
#include <puppet/cast.hpp>
#include <sstream>

using namespace std;
using namespace puppet::lexer;
using boost::variant;
using boost::optional;

namespace puppet { namespace ast {

    struct hostname_visitor : boost::static_visitor<>
    {
        template <typename T>
        void operator()(T const& part)
        {
            if (_ss.tellp() > 0) {
                _ss << '.';
            }
            _ss << part.value;

            if (!_position) {
                _position = part.position;
            }
        }
        std::string result() const
        {
            return _ss.str();
        }

        optional<lexer::position> const& position() const
        {
            return _position;
        }

     private:
        ostringstream _ss;
        optional<lexer::position> _position;
    };

    hostname::hostname() :
        regex(false)
    {
    }

    hostname::hostname(ast::defaulted defaulted) :
        position(rvalue_cast(defaulted.position)),
        regex(false)
    {
    }

    hostname::hostname(vector<variant<name, bare_word, number>> const& parts) :
        regex(false)
    {
        hostname_visitor visitor;
        for (auto const& part : parts) {
            boost::apply_visitor(visitor, part);
        }
        value = visitor.result();
        if (visitor.position()) {
            position = *visitor.position();
        }
    }

    hostname::hostname(ast::string name) :
        position(rvalue_cast(name.position)),
        value(rvalue_cast(name.value)),
        regex(false)
    {
    }

    hostname::hostname(ast::regex regx) :
        position(rvalue_cast(regx.position)),
        value(rvalue_cast(regx.value)),
        regex(true)
    {
    }

    ostream& operator<<(ostream& os, hostname const& name)
    {
        if (!name.regex && name.value.empty()) {
            os << "default";
        } else if (name.regex) {
            os << '/' << name.value << '/';
        } else {
            os << name.value;
        }
        return os;
    }

    node_definition_expression::node_definition_expression()
    {
    }

    node_definition_expression::node_definition_expression(lexer::position position, vector<hostname> names, optional<vector<expression>> body) :
        position(rvalue_cast(position)),
        names(rvalue_cast(names)),
        body(rvalue_cast(body))
    {
    }

    ostream& operator<<(ostream& os, node_definition_expression const& expr)
    {
        if (expr.names.empty()) {
            return os;
        }

        os << "node ";
        pretty_print(os, expr.names, ", ");
        os << " { ";
        pretty_print(os, expr.body, "; ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast
