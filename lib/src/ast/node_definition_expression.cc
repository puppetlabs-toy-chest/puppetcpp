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
            _ss << part.value();

            if (!_position) {
                _position = part.position();
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
        _regex(false)
    {
    }

    hostname::hostname(ast::defaulted const& defaulted) :
        _position(defaulted.position()),
        _regex(false)
    {
    }

    hostname::hostname(vector<variant<name, bare_word, number>> const& parts) :
        _regex(false)
    {
        hostname_visitor visitor;
        for (auto const& part : parts) {
            boost::apply_visitor(visitor, part);
        }
        _value = visitor.result();
        if (visitor.position()) {
            _position = *visitor.position();
        }
    }

    hostname::hostname(ast::string const& name) :
        _position(name.position()),
        _value(name.value()),
        _regex(false)
    {
    }

    hostname::hostname(ast::regex const& regx) :
        _position(regx.position()),
        _value(regx.value()),
        _regex(true)
    {
    }

    std::string const& hostname::value() const
    {
        return _value;
    }

    bool hostname::regex() const
    {
        return _regex;
    }

    bool hostname::is_default() const
    {
        return !_regex && _value.empty();
    }

    lexer::position const& hostname::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, hostname const& name)
    {
        if (name.is_default()) {
            os << "default";
        } else if (name.regex()) {
            os << '/' << name.value() << '/';
        } else {
            os << name.value();
        }
        return os;
    }

    node_definition_expression::node_definition_expression()
    {
    }

    node_definition_expression::node_definition_expression(lexer::position position, vector<hostname> names, optional<vector<expression>> body) :
        _position(rvalue_cast(position)),
        _names(rvalue_cast(names)),
        _body(rvalue_cast(body))
    {
    }

    vector<hostname> const& node_definition_expression::names() const
    {
        return _names;
    }

    optional<vector<expression>> const& node_definition_expression::body() const
    {
        return _body;
    }

    lexer::position const& node_definition_expression::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, node_definition_expression const& stmt)
    {
        if (stmt.names().empty()) {
            return os;
        }

        os << "node ";
        pretty_print(os, stmt.names(), ", ");
        os << " { ";
        pretty_print(os, stmt.body(), "; ");
        os << " }";
        return os;
    }

}}  // namespace puppet::ast
