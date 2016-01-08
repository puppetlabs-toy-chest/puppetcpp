#include <puppet/compiler/evaluation/dispatcher.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/functions/alert.hpp>
#include <puppet/compiler/evaluation/functions/assert_type.hpp>
#include <puppet/compiler/evaluation/functions/contain.hpp>
#include <puppet/compiler/evaluation/functions/crit.hpp>
#include <puppet/compiler/evaluation/functions/debug.hpp>
#include <puppet/compiler/evaluation/functions/defined.hpp>
#include <puppet/compiler/evaluation/functions/each.hpp>
#include <puppet/compiler/evaluation/functions/emerg.hpp>
#include <puppet/compiler/evaluation/functions/err.hpp>
#include <puppet/compiler/evaluation/functions/fail.hpp>
#include <puppet/compiler/evaluation/functions/filter.hpp>
#include <puppet/compiler/evaluation/functions/include.hpp>
#include <puppet/compiler/evaluation/functions/info.hpp>
#include <puppet/compiler/evaluation/functions/inline_epp.hpp>
#include <puppet/compiler/evaluation/functions/map.hpp>
#include <puppet/compiler/evaluation/functions/notice.hpp>
#include <puppet/compiler/evaluation/functions/realize.hpp>
#include <puppet/compiler/evaluation/functions/reduce.hpp>
#include <puppet/compiler/evaluation/functions/require.hpp>
#include <puppet/compiler/evaluation/functions/split.hpp>
#include <puppet/compiler/evaluation/functions/tag.hpp>
#include <puppet/compiler/evaluation/functions/tagged.hpp>
#include <puppet/compiler/evaluation/functions/versioncmp.hpp>
#include <puppet/compiler/evaluation/functions/warning.hpp>
#include <puppet/compiler/evaluation/functions/with.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation {

    void dispatcher::add_builtin_functions()
    {
        // Add the built-in functions
        add(functions::alert::create_descriptor());
        add(functions::assert_type::create_descriptor());
        add(functions::contain::create_descriptor());
        add(functions::crit::create_descriptor());
        add(functions::debug::create_descriptor());
        add(functions::defined::create_descriptor());
        add(functions::each::create_descriptor());
        add(functions::emerg::create_descriptor());
        add(functions::err::create_descriptor());
        add(functions::fail::create_descriptor());
        add(functions::filter::create_descriptor());
        add(functions::include::create_descriptor());
        add(functions::info::create_descriptor());
        add(functions::inline_epp::create_descriptor());
        add(functions::map::create_descriptor());
        add(functions::notice::create_descriptor());
        add(functions::realize::create_descriptor());
        add(functions::reduce::create_descriptor());
        add(functions::require::create_descriptor());
        add(functions::split::create_descriptor());
        add(functions::tag::create_descriptor());
        add(functions::tagged::create_descriptor());
        add(functions::versioncmp::create_descriptor());
        add(functions::warning::create_descriptor());
        add(functions::with::create_descriptor());
    }

    void dispatcher::add(functions::descriptor descriptor)
    {
        if (descriptor.name().empty()) {
            throw runtime_error("cannot add a function with an empty name to the dispatcher.");
        }
        if (!descriptor.dispatchable()) {
            throw runtime_error("cannot add a function that is not dispatchable to the dispatcher.");
        }
        string name = descriptor.name();
        if (!_functions.emplace(rvalue_cast(name), rvalue_cast(descriptor)).second) {
            throw runtime_error((boost::format("function '%1%' already exists in the dispatcher.") % name).str());
        }
    }

    functions::descriptor* dispatcher::find(string const& name)
    {
        return const_cast<functions::descriptor*>(static_cast<dispatcher const*>(this)->find(name));
    }

    functions::descriptor const* dispatcher::find(string const& name) const
    {
        auto it = _functions.find(name);
        if (it == _functions.end()) {
            return nullptr;
        }
        return &it->second;
    }

    values::value dispatcher::dispatch(functions::call_context& context) const
    {
        // Find the requested function
        auto descriptor = find(context.name().value);
        if (!descriptor) {
            throw evaluation_exception((boost::format("unknown function '%1%'.") % context.name()).str(), context.name());
        }
        return descriptor->dispatch(context);
    }

    void dispatcher::fallback(fallback_type fallback)
    {
        _fallback = rvalue_cast(fallback);
    }

}}}  // namespace puppet::compiler::evaluation
