#include <puppet/compiler/evaluation/functions/tagged.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/catalog.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    value tagged::operator()(function_call_context& context) const
    {
        // Check the empty arguments count
        auto& arguments = context.arguments();
        if (arguments.empty()) {
            throw evaluation_exception((boost::format("expected at least one argument to '%1%' function.") % context.name()).str(), context.call_site());
        }

        // Get the current scope's resource
        auto resource = context.context().current_scope()->resource();
        if (!resource) {
            return false;
        }

        auto tags = resource->calculate_tags();

        // Make sure all given arguments are in the tag set
        for (size_t i = 0; i < arguments.size(); ++i) {
            auto& argument = *arguments[i];
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
    }

}}}}  // namespace puppet::compiler::evaluation::functions
