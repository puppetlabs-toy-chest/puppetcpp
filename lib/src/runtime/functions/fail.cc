#include <puppet/runtime/functions/fail.hpp>
#include <puppet/runtime/expression_evaluator.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace runtime { namespace functions {

    value fail::operator()(call_context& context) const
    {
        ostringstream ss;
        ss << "evaluation failed";
        if (!context.arguments().empty()) {
            ss << ": ";
            join(ss, context.arguments(), " ");
        }
        throw evaluation_exception(context.position(), ss.str());
    }

}}}  // namespace puppet::runtime::functions
