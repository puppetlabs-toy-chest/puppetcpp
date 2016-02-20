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

    ostream& operator<<(ostream& os, lexer::position const& position)
    {
        os << "(" << position.offset() << ", " << position.line() << ")";
        return os;
    }

    bool operator==(position const& left, position const& right)
    {
        return left.offset() == right.offset() && left.line() == right.line();
    }

    bool operator!=(position const& left, position const& right)
    {
        return !(left == right);
    }

}}}  // namespace puppet::compiler::lexer
