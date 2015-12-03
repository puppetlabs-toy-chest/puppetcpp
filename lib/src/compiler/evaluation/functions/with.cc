#include <puppet/compiler/evaluation/functions/with.hpp>
#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    value with::operator()(function_call_context& context) const
    {
        if (!context.lambda()) {
            throw evaluation_exception((boost::format("expected a lambda to '%1%' function but one was not given.") % context.name()).str(), context.name());
        }

        try {
            return context.yield_without_catch(context.arguments());
        } catch (argument_exception const& ex) {
            throw evaluation_exception(ex.what(), context.argument_context(ex.index()));
        }
    }

}}}}  // namespace puppet::compiler::evaluation::functions
