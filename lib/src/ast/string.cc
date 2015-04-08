#include <puppet/ast/string.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    string::string() :
        _quote(0),
        _interpolated(false),
        _margin(0),
        _remove_break(false)
    {
    }

    std::string const& string::value() const
    {
        return _value;
    }

    std::string const& string::escapes() const
    {
        return _escapes;
    }

    char string::quote() const
    {
        return _quote;
    }

    bool string::interpolated() const
    {
        return _interpolated;
    }

    std::string const& string::format() const
    {
        return _format;
    }

    int string::margin() const
    {
        return _margin;
    }

    bool string::remove_break() const
    {
        return _remove_break;
    }

    token_position const& string::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, string const& str)
    {
        os << (str.interpolated() ? '"' : '\'') << str.value() << (str.interpolated() ? '"' : '\'');
        return os;
    }

}}  // namespace puppet::ast
