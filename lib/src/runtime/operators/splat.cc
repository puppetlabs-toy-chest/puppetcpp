#include <puppet/runtime/operators/splat.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    value splat::operator()(unary_context& context) const
    {
        if (as<values::array>(context.operand())) {
            return mutate(context.operand());
        }
        return to_array(dereference(context.operand()));
    }

}}}  // namespace puppet::runtime::operators
