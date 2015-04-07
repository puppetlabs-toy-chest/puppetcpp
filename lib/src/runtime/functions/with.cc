#include <puppet/runtime/functions/with.hpp>
#include <puppet/runtime/expression_evaluator.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace runtime { namespace functions {

    value with::operator()(context& ctx, token_position const& position, array& arguments, runtime::yielder& yielder) const
    {
        return yielder.yield(arguments);
    }

}}}  // namespace puppet::runtime::functions
