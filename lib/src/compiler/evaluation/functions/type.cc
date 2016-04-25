#include <puppet/compiler/evaluation/functions/type.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    descriptor type::create_descriptor()
    {
        functions::descriptor descriptor{ "type" };

        descriptor.add("Callable[Any, String, 1, 2]", [](call_context& context) -> values::value {
            if (context.arguments().size() == 2) {
                auto& method = context.argument(1).require<string>();
                if (method == "reduced") {
                    return context.argument(0).infer_type();
                }
                if (method == "generalized") {
                    return context.argument(0).infer_type().generalize();
                }
                if (method != "detailed") {
                    throw evaluation_exception(
                        (boost::format("'%1%' is not a valid type inference method: expected 'generalized', 'reduced', or 'detailed'.") %
                         method
                        ).str(),
                        context.argument_context(1),
                        context.context().backtrace()
                    );
                }
            }
            return context.argument(0).infer_type(true);
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
