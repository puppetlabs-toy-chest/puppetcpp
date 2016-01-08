#include <puppet/compiler/evaluation/functions/tagged.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/catalog.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    descriptor tagged::create_descriptor()
    {
        functions::descriptor descriptor{ "tagged" };

        descriptor.add("Callable[Variant[String, Array[Any]], 1]", [](call_context& context) {
            // Get the current scope's resource
            auto resource = context.context().current_scope()->resource();
            if (!resource) {
                return false;
            }

            auto tags = resource->calculate_tags();

            // Make sure all given arguments are in the tag set
            auto& arguments = context.arguments();
            for (size_t i = 0; i < arguments.size(); ++i) {
                auto& argument = context.argument(i);
                bool matches = true;
                if (!argument.move_as<string>([&](string tag) {
                    if (tags.find(&tag) == tags.end()) {
                        matches = false;
                        return false;
                    }
                    return true;
                })) {
                    throw evaluation_exception((boost::format("expected %1% or array of %1% but found %2%.") % types::string::name() % argument.get_type()).str(), context.argument_context(i));
                }
                if (!matches) {
                    return false;
                }
            }
            return true;
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
