#include <puppet/compiler/evaluation/operators/not_equals.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    value not_equals::operator()(binary_operator_context const& context) const
    {
        return context.left() != context.right();
    }

}}}}  // namespace puppet::compiler::evaluation::operators
