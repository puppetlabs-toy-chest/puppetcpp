#include <puppet/lexer/string_token.hpp>

using namespace std;

namespace puppet { namespace lexer {

    string_token::string_token() :
        _interpolated(true),
        _escaped(true)
    {
    }

    string_token::string_token(token_position position, string text, string format, bool interpolated, bool escaped) :
        _position(std::move(position)),
        _text(std::move(text)),
        _format(std::move(format)),
        _interpolated(interpolated),
        _escaped(escaped)
    {
    }

    token_position const& string_token::position() const
    {
        return _position;
    }

    string const& string_token::text() const
    {
        return _text;
    }

    string const& string_token::format() const
    {
        return _format;
    }

    bool string_token::interpolated() const
    {
        return _interpolated;
    }

    bool string_token::escaped() const
    {
        return _escaped;
    }

    ostream& operator<<(ostream& os, string_token const& token)
    {
        os << (token.interpolated() ? '"' : '\'') << token.text() << (token.interpolated() ? '"' : '\'');
        return os;
    }

}}  // namespace puppet::lexer
