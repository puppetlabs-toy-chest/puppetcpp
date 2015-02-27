#include <puppet/ast/string.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    string::string()
    {
    }

    string::string(string_token& token) :
        _position(token.position()),
        _value(std::move(token.text())),
        _format(std::move(token.format())),
        _interpolated(token.interpolated()),
        _escaped(token.escaped())
    {
    }

    std::string const& string::value() const
    {
        return _value;
    }

    std::string& string::value()
    {
        return _value;
    }

    std::string const& string::format() const
    {
        return _format;
    }

    std::string& string::format()
    {
        return _format;
    }

    bool string::interpolated() const
    {
        return _interpolated;
    }

    bool& string::interpolated()
    {
        return _interpolated;
    }

    bool string::escaped() const
    {
        return _escaped;
    }

    bool& string::escaped()
    {
        return _escaped;
    }

    token_position const& string::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, string const& str)
    {
        os << (str.interpolated() ? '"' : '\'');
        os << str.value();
        os << (str.interpolated() ? '"' : '\'');
        return os;
    }

}}  // namespace puppet::ast
