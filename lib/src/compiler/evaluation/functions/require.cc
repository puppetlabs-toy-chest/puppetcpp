#include <puppet/compiler/evaluation/functions/require.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    // Forward declare this function (implemented in include.cc)
    void declare_class(call_context& context, boost::optional<compiler::relationship> relationship = boost::none);

    descriptor require::create_descriptor()
    {
        functions::descriptor descriptor{ "require" };

        descriptor.add("Callable[Variant[String, Array[Any], Class, Resource], 1]", [](call_context& context) {
            declare_class(context, relationship::require);
            return values::undef{};
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
