#include <puppet/runtime/functions/fail.hpp>
#include <puppet/runtime/expression_evaluator.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace runtime { namespace functions {

    value fail::operator()(context& ctx, token_position const& position, array& arguments, runtime::yielder& yielder) const
    {
        ostringstream ss;
        ss << "evaluation failed";
        if (!arguments.empty()) {
            ss << ": ";
            join(ss, arguments, " ");
        }
        throw evaluation_exception(position, ss.str());
    }

}}}  // namespace puppet::runtime::functions
