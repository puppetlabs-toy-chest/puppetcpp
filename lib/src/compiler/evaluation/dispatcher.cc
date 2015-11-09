#include <puppet/compiler/evaluation/dispatcher.hpp>
#include <puppet/compiler/evaluation/functions/assert_type.hpp>
#include <puppet/compiler/evaluation/functions/declare.hpp>
#include <puppet/compiler/evaluation/functions/defined.hpp>
#include <puppet/compiler/evaluation/functions/each.hpp>
#include <puppet/compiler/evaluation/functions/fail.hpp>
#include <puppet/compiler/evaluation/functions/filter.hpp>
#include <puppet/compiler/evaluation/functions/function_call_context.hpp>
#include <puppet/compiler/evaluation/functions/log.hpp>
#include <puppet/compiler/evaluation/functions/map.hpp>
#include <puppet/compiler/evaluation/functions/realize.hpp>
#include <puppet/compiler/evaluation/functions/reduce.hpp>
#include <puppet/compiler/evaluation/functions/split.hpp>
#include <puppet/compiler/evaluation/functions/tag.hpp>
#include <puppet/compiler/evaluation/functions/versioncmp.hpp>
#include <puppet/compiler/evaluation/functions/with.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation {

    values::value dispatcher::dispatch(functions::function_call_context& context) const
    {
        // Keep in alphabetical order
        static const unordered_map<string, function<values::value(functions::function_call_context&)>> builtin_functions {
            { "alert",          functions::log(logging::level::alert) },
            { "assert_type",    functions::assert_type() },
            { "contain",        functions::declare(relationship::contains) },
            { "crit",           functions::log(logging::level::critical) },
            { "debug",          functions::log(logging::level::debug) },
            { "defined",        functions::defined() },
            { "each",           functions::each() },
            { "emerg",          functions::log(logging::level::emergency) },
            { "err",            functions::log(logging::level::error) },
            { "fail",           functions::fail() },
            { "filter",         functions::filter() },
            { "include",        functions::declare() },
            { "info",           functions::log(logging::level::info) },
            { "map",            functions::map() },
            { "notice",         functions::log(logging::level::notice) },
            { "realize",        functions::realize() },
            { "reduce",         functions::reduce() },
            { "require",        functions::declare(relationship::require) },
            { "split",          functions::split() },
            { "tag",            functions::tag() },
            { "versioncmp",     functions::versioncmp() },
            { "warning",        functions::log(logging::level::warning) },
            { "with",           functions::with() },
        };

        // TODO: check a local collection of "registered" functions first

        // Find a builtin function
        auto it = builtin_functions.find(context.name());
        if (it == builtin_functions.end()) {
            throw evaluation_exception((boost::format("unknown function '%1%'.") % context.name()).str(), context.call_site());
        }
        return it->second(context);
    }

}}}  // namespace puppet::compiler::evaluation
