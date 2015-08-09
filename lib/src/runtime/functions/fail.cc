#include <puppet/runtime/functions/fail.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    value fail::operator()(call_context& context) const
    {
        ostringstream ss;
        ss << "evaluation failed";
        if (!context.arguments().empty()) {
            ss << ": ";
            join(ss, context.arguments(), " ");
        }
        throw context.evaluator().create_exception(context.position(), ss.str());
    }

}}}  // namespace puppet::runtime::functions
