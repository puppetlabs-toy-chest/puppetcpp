#include <puppet/runtime/functions/assert_type.hpp>
#include <puppet/runtime/expression_evaluator.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace functions {

    value assert_type::operator()(call_context& context) const
    {
        // Check the argument count
        auto& arguments = context.arguments();
        if (arguments.size() != 2) {
            throw evaluation_exception(arguments.size() > 2 ? context.position(2) : context.position(), (boost::format("expected 2 arguments to 'assert_type' function but %1% were given.") % arguments.size()).str());
        }

        // First argument should be a type (TODO: should accept a string that is a type name too)
        auto type = boost::get<values::type>(&dereference(arguments[0]));
        if (!type) {
            throw evaluation_exception(context.position(0), (boost::format("expected %1% for first argument but found %2%.") % types::type::name() % get_type(arguments[0])).str());
        }

        // If the value is an instance of the type, return it
        if (is_instance(arguments[1], *type)) {
            return std::move(arguments[1]);
        }

        // Otherwise, a lambda is required
        if (!context.yielder().lambda_given()) {
            throw evaluation_exception(context.position(1), (boost::format("type assertion failure: expected %1% but found %2%.") % *type % get_type(arguments[1])).str());
        }

        arguments[1] = get_type(arguments[1]);
        return context.yielder().yield(arguments);
    }

}}}  // namespace puppet::runtime::functions
