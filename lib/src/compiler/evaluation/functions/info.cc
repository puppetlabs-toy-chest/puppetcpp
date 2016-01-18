#include <puppet/compiler/evaluation/functions/info.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/scope.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    // Forward declare shared implementation in alert.cc
    values::value log(logging::logger& logger, scope const& current, logging::level level, values::array const& arguments);

    descriptor info::create_descriptor()
    {
        functions::descriptor descriptor{ "info" };

        descriptor.add("Callable", [](call_context& context) {
            auto& evaluation_context = context.context();
            return log(evaluation_context.node().logger(), *evaluation_context.current_scope(), logging::level::info, context.arguments());
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
