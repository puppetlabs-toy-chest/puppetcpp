#include <puppet/compiler/evaluation/functions/log.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    log::log(logging::level level) :
        _level(level)
    {
    }

    value log::operator()(function_call_context& context) const
    {
        auto& logger = context.context().node().logger();

        // Format the message based on the arguments
        ostringstream ss;
        context.arguments().join(ss, " ");
        auto message = ss.str();

        // Log the message
        if (logger.would_log(_level)) {
            logger.log(_level, (boost::format("%1%: %2%") % *context.context().current_scope() % message).str(), false /* errors are not failures */);
        }
        return message;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
