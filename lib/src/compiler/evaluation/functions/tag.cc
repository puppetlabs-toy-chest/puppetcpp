#include <puppet/compiler/evaluation/functions/tag.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    descriptor tag::create_descriptor()
    {
        functions::descriptor descriptor{ "tag" };

        descriptor.add("Callable[Variant[String, Array[Any]], 1]", [](call_context& context) {
            // Get the current scope's resource
            auto resource = context.context().current_scope()->resource();
            if (!resource) {
                throw evaluation_exception("the current scope has no associated resource and cannot be tagged.", context.name());
            }

            // Add the tags to the resource
            auto& arguments = context.arguments();
            for (size_t i = 0; i < arguments.size(); ++i) {
                auto& argument = context.argument(i);
                if (!argument.move_as<string>([&](string tag) {
                    resource->tag(rvalue_cast(tag));
                    return true;
                })) {
                    throw evaluation_exception(
                        (boost::format("expected %1% or %2%[%1%] of %1% but found %2%.") %
                         types::string::name() %
                         types::array::name() %
                         argument.get_type()
                        ).str(),
                        context.argument_context(i));
                }
            }
            return undef();
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
