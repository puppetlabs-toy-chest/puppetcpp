#include <puppet/runtime/operators/splat.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    value splat::operator()(unary_context& context) const
    {
        return values::to_array(context.operand());
    }

}}}  // namespace puppet::runtime::operators
