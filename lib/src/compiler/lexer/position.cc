#include <puppet/compiler/lexer/position.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace compiler { namespace lexer {

    position::position() :
        _offset(0),
        _line(0)
    {
    }

    position::position(size_t offset, size_t line) :
        _offset(offset),
        _line(line)
    {
    }

    size_t position::offset() const
    {
        return _offset;
    }

    size_t position::line() const
    {
        return _line;
    }

    void position::increment(bool newline)
    {
        if (newline) {
            ++_line;
        }

        ++_offset;
    }

    range::range(position begin, position end) :
        _begin(rvalue_cast(begin)),
        _end(rvalue_cast(end))
    {
    }

    range::range(position begin, size_t length) :
        _begin(rvalue_cast(begin))
    {
        _end = position{ _begin.offset() + length, _begin.line() };
    }

    position const& range::begin() const
    {
        return _begin;
    }

    void range::begin(position begin)
    {
        _begin = rvalue_cast(begin);
    }

    position const& range::end() const
    {
        return _end;
    }

    void range::end(position end)
    {
        _end = rvalue_cast(end);
    }

    size_t range::length() const
    {
        return _end.offset() - _begin.offset();
    }

    ostream& operator<<(ostream& os, lexer::position const& position)
    {
        os << "(" << position.offset() << ", " << position.line() << ")";
        return os;
    }

}}}  // namespace puppet::compiler::lexer