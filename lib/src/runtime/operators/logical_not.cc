#include <puppet/runtime/operators/logical_not.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    value logical_not::operator()(unary_context& context) const
    {
        return !is_truthy(context.operand());
    }

}}}  // namespace puppet::runtime::operators
