#include <puppet/runtime/operators/logical_or.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    value logical_or::operator()(binary_context& context) const
    {
        return is_truthy(context.left()) || is_truthy(context.right());
    }

}}}  // namespace puppet::runtime::operators
