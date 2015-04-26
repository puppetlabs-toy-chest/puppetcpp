#include <puppet/runtime/functions/logging.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    logging_function::logging_function(logging::level lvl) :
        _level(lvl)
    {
    }

    value logging_function::operator()(call_context& context) const
    {
        // Format the message based on the arguments
        ostringstream ss;
        join(ss, context.arguments(), " ");
        string message = ss.str();

        context.evaluation_context().logger().log(_level, "%1%: %2%", context.evaluation_context().current(), message);
        return message;
    }

}}}  // namespace puppet::runtime::functions
