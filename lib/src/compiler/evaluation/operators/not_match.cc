#include <puppet/compiler/evaluation/operators/not_match.hpp>
#include <puppet/compiler/evaluation/operators/match.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    value not_match::operator()(binary_operator_context const& context) const
    {
        return !match()(context).is_truthy();
    }

}}}}  // namespace puppet::compiler::evaluation::operators
