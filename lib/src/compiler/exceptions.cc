#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace compiler {

    parse_exception::parse_exception(lexer::position position, string const& message) :
        runtime_error(message),
        _position(rvalue_cast(position))
    {
    }

    lexer::position const& parse_exception::position() const
    {
        return _position;
    }

    compilation_exception::compilation_exception(string const& message, string path, size_t line, size_t column, string text) :
        runtime_error(message),
        _path(rvalue_cast(path)),
        _line(line),
        _column(column),
        _text(rvalue_cast(text))
    {
    }

    string const& compilation_exception::path() const
    {
        return _path;
    }

    size_t compilation_exception::line() const
    {
        return _line;
    }

    size_t compilation_exception::column() const
    {
        return _column;
    }

    string const& compilation_exception::text() const
    {
        return _text;
    }

    settings_exception::settings_exception(string const& message) :
        runtime_error(message)
    {
    }

}}  // namespace puppet::compiler