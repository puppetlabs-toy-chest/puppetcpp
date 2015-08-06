#include <puppet/runtime/functions/with.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    value with::operator()(call_context& context) const
    {
        return context.lambda().execute(context.arguments(), [&](size_t index, string message) {
            return context.evaluator().create_exception(context.position(index), rvalue_cast(message));
        });
    }

}}}  // namespace puppet::runtime::functions
