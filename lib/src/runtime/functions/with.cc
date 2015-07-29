#include <puppet/runtime/functions/with.hpp>
#include <puppet/runtime/expression_evaluator.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    value with::operator()(call_context& context) const
    {
        return context.lambda().execute(context.arguments(), [&](size_t index) {
            return context.position(index);
        });
    }

}}}  // namespace puppet::runtime::functions
