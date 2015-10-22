#include <puppet/compiler/evaluation/functions/fail.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    value fail::operator()(function_call_context& context) const
    {
        ostringstream ss;
        ss << "evaluation failed";
        if (!context.arguments().empty()) {
            ss << ": ";
            context.arguments().join(ss, " ");
        }
        throw evaluation_exception(ss.str(), context.call_site());
    }

}}}}  // namespace puppet::compiler::evaluation::functions
