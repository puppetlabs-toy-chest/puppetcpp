#include <puppet/compiler/evaluation/operators/logical_and.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    value logical_and::operator()(binary_operator_context const& context) const
    {
        return context.left().is_truthy() && context.right().is_truthy();
    }

}}}}  // namespace puppet::compiler::evaluation::operators
