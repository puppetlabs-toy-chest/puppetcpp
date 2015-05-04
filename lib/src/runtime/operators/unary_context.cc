#include <puppet/runtime/operators/unary_context.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    unary_context::unary_context(context& evaluation_context, value& operand, token_position const& position) :
        _operand(operand),
        _position(position),
        _evaluation_context(evaluation_context)
    {
    }

    context& unary_context::evaluation_context()
    {
        return _evaluation_context;
    }

    context const& unary_context::evaluation_context() const
    {
        return _evaluation_context;
    }

    value& unary_context::operand()
    {
        return _operand;
    }

    value const& unary_context::operand() const
    {
        return _operand;
    }

    token_position const& unary_context::position() const
    {
        return _position;
    }



}}}  // namespace puppet::runtime::operators
