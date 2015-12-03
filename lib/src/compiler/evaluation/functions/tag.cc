#include <puppet/compiler/evaluation/functions/tag.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/catalog.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    value tag::operator()(function_call_context& context) const
    {
        // Check the empty arguments count
        auto& arguments = context.arguments();
        if (arguments.empty()) {
            throw evaluation_exception((boost::format("expected at least one argument to '%1%' function.") % context.name()).str(), context.name());
        }

        // Get the current scope's resource
        auto resource = context.context().current_scope()->resource();
        if (!resource) {
            throw evaluation_exception("the current scope has no associated resource and cannot be tagged.", context.name());
        }

        // Add the tags to the resource
        for (size_t i = 0; i < arguments.size(); ++i) {
            auto& argument = *arguments[i];
            if (!argument.move_as<string>([&](string tag) {
                resource->tag(rvalue_cast(tag));
                return true;
            })) {
                throw evaluation_exception((boost::format("expected %1% or array of %1% but found %2%.") % types::string::name() % argument.get_type()).str(), context.argument_context(i));
            }
        }
        return undef();
    }

}}}}  // namespace puppet::compiler::evaluation::functions
