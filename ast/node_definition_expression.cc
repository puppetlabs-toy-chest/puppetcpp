#include "node_definition_expression.hpp"
#include "expression_def.hpp"
#include "utility.hpp"
#include <sstream>

using namespace std;
using namespace puppet::lexer;
using boost::variant;
using boost::optional;

namespace puppet { namespace ast {

    struct hostname_visitor : boost::static_visitor<>
    {
        void operator()(name const& n)
        {
            if (_ss.tellp() > 0) {
                _ss << '.';
            }
            _ss << n.value();

            if (!_position) {
                _position = n.position();
            }
        }

        void operator()(number const& n)
        {
            if (_ss.tellp() > 0) {
                _ss << '.';
            }
            _ss << n.value();

            if (!_position) {
                _position = n.position();
            }
        }

        std::string result() const
        {
            return _ss.str();
        }

        optional<token_position> const& position() const
        {
            return _position;
        }

     private:
        ostringstream _ss;
        optional<token_position> _position;
    };

    hostname::hostname() :
        _regex(false)
    {
    }

    hostname::hostname(token_position position) :
        _position(position),
        _regex(false)
    {
    }

    hostname::hostname(vector<variant<name, number>> const& parts) :
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

    hostname::hostname(string name) :
        _value(std::move(name.value())),
        _regex(false)
    {
    }

    hostname::hostname(struct regex name) :
        _value(std::move(name.value())),
        _regex(true)
    {
    }

    std::string const& hostname::value() const
    {
        return _value;
    }

    std::string& hostname::value()
    {
        return _value;
    }

    bool hostname::regex() const
    {
        return _regex;
    }

    bool& hostname::regex()
    {
        return _regex;
    }

    bool hostname::is_default() const
    {
        return _value.empty();
    }

    token_position const& hostname::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, hostname const& name)
    {
        if (name.is_default()) {
            os << "default";
        } else {
            os << name.value();
        }
        return os;
    }

    node_definition_expression::node_definition_expression()
    {
    }

    node_definition_expression::node_definition_expression(token_position position, vector<hostname> names, optional<vector<expression>> body) :
        _position(std::move(position)),
        _names(std::move(names)),
        _body(std::move(body))
    {
    }

    vector<hostname> const& node_definition_expression::names() const
    {
        return _names;
    }

    vector<hostname>& node_definition_expression::names()
    {
        return _names;
    }

    optional<vector<expression>> const& node_definition_expression::body() const
    {
        return _body;
    }

    optional<vector<expression>>& node_definition_expression::body()
    {
        return _body;
    }

    token_position const& node_definition_expression::position() const
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
