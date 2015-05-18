#include <puppet/runtime/operators/unary_context.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    unary_context::unary_context(expression_evaluator& evaluator, value& operand, token_position const& position) :
        _evaluator(evaluator),
        _operand(operand),
        _position(position)
    {
    }

    expression_evaluator& unary_context::evaluator()
    {
        return _evaluator;
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
