#include <puppet/runtime/operators/binary_context.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    binary_context::binary_context(context& evaluation_context, value& left, token_position const& left_position, value& right, token_position const& right_position) :
        _left(left),
        _left_position(left_position),
        _right(right),
        _right_position(right_position),
        _evaluation_context(evaluation_context)
    {
    }

    context& binary_context::evaluation_context()
    {
        return _evaluation_context;
    }

    context const& binary_context::evaluation_context() const
    {
        return _evaluation_context;
    }

    value& binary_context::left()
    {
        return _left;
    }

    value const& binary_context::left() const
    {
        return _left;
    }

    token_position const& binary_context::left_position() const
    {
        return _left_position;
    }

    value& binary_context::right()
    {
        return _right;
    }

    value const& binary_context::right() const
    {
        return _right;
    }

    token_position const& binary_context::right_position() const
    {
        return _right_position;
    }

}}}  // namespace puppet::runtime::operators
