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

        auto& evaluator = context.evaluator();
        evaluator.logger().log(_level, "%1%: %2%", evaluator.scope(), message);
        return message;
    }

}}}  // namespace puppet::runtime::functions
