#include <puppet/ast/variable.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    variable::variable()
    {
    }

    variable::variable(string name, lexer::token_position position) :
        _position(std::move(position)),
        _name(std::move(name))
    {

    }

    string const& variable::name() const
    {
        return _name;
    }

    token_position const& variable::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, variable const& var)
    {
        os << "$" << var.name();
        return os;
    }

}}  // namespace puppet::ast
