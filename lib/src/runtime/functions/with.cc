#include <puppet/runtime/functions/with.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    value with::operator()(call_context& context) const
    {
        try {
            return context.yield_without_catch(context.arguments());
        } catch (argument_exception const& ex) {
            throw context.evaluator().create_exception(context.position(ex.index()), ex.what());
        }
    }

}}}  // namespace puppet::runtime::functions
