#include <puppet/compiler/evaluation/functions/alert.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/scope.hpp>
#include <puppet/logging/logger.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    values::value log(logging::logger& logger, scope const& current, logging::level level, values::array const& arguments)
    {
        // Format the message based on the arguments
        ostringstream ss;
        arguments.join(ss, " ");
        auto message = ss.str();

        // Log the message
        if (logger.would_log(level)) {
            logger.log(level, (boost::format("%1%: %2%") % current % message).str());
        }
        return message;
    }

    descriptor alert::create_descriptor()
    {
        functions::descriptor descriptor{ "alert" };

        descriptor.add("Callable", [](call_context& context) {
            auto& evaluation_context = context.context();
            return log(evaluation_context.node().logger(), *evaluation_context.calling_scope(), logging::level::alert, context.arguments());
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
