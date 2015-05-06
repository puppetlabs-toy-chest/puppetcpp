#include <puppet/runtime/operators/not_match.hpp>
#include <puppet/runtime/operators/match.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    value not_match::operator()(binary_context& context) const
    {
        return !is_truthy(match()(context));
    }

}}}  // namespace puppet::runtime::operators
