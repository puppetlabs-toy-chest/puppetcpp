#include <puppet/compiler/evaluation/functions/with.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    descriptor with::create_descriptor()
    {
        functions::descriptor descriptor{ "with" };

        descriptor.add("Callable[0, default, Callable]", [](call_context& context) {
            try {
                return context.yield_without_catch(context.arguments());
            } catch (argument_exception const& ex) {
                throw evaluation_exception(ex.what(), context.argument_context(ex.index()));
            }
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
