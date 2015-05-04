#include <puppet/runtime/operators/not_equals.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    value not_equals::operator()(binary_context& context) const
    {
        return !values::equals(context.left(), context.right());
    }

}}}  // namespace puppet::runtime::operators
