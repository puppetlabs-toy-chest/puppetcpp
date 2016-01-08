#include <puppet/compiler/evaluation/functions/fail.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    descriptor fail::create_descriptor()
    {
        functions::descriptor descriptor{ "fail" };

        descriptor.add("Callable", [](call_context& context) -> value {
            ostringstream ss;
            ss << "evaluation failed";
            if (!context.arguments().empty()) {
                ss << ": ";
                context.arguments().join(ss, " ");
            }
            throw evaluation_exception(ss.str(), context.name());
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
