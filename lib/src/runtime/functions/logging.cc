#include <puppet/runtime/functions/logging.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace runtime { namespace functions {

    logging_function::logging_function(logging::level lvl) :
        _level(lvl)
    {
    }

    value logging_function::operator()(context& ctx, token_position const& position, array& arguments, runtime::yielder& yielder) const
    {
        // Format the message based on the arguments
        ostringstream ss;
        join(ss, arguments, " ");
        string message = ss.str();

        ctx.logger().log(_level, "%1%: %2%", ctx.current(), message);
        return arguments.empty() ? value() : message;
    }

}}}  // namespace puppet::runtime::functions
