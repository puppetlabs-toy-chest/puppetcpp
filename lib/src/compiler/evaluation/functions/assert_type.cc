#include <puppet/compiler/evaluation/functions/assert_type.hpp>
#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static bool validate_type(function_call_context& context, values::type const& type, values::value& instance, ast::context const& argument_context)
    {
        if (type.is_instance(instance)) {
            return true;
        }

        // Otherwise, a lambda is required
        if (!context.lambda()) {
            throw evaluation_exception((boost::format("type assertion failure: expected %1% but found %2%.") % type % instance.get_type()).str(), argument_context);
        }
        return false;
    }

    values::value assert_type::operator()(function_call_context& context) const
    {
        // Check the argument count
        auto& arguments = context.arguments();
        size_t count = arguments.size();
        if (count != 2) {
            throw evaluation_exception((boost::format("expected 2 arguments to '%1%' function but %2% were given.") % context.name() % count).str(), count > 2 ? context.argument_context(2) : context.name());
        }

        auto& first = context.argument(0);
        auto& second = context.argument(1);

        // First argument should be a type or a string
        if (auto type = first.as<values::type>()) {
            // If the value is an instance of the type, return it
            if (validate_type(context, *type, second, context.argument_context(1))) {
                return rvalue_cast(second);
            }
        } else if (auto str = first.as<string>()) {
            auto type = values::type::parse(context.context(), *str);
            if (!type) {
                throw evaluation_exception((boost::format("the expression '%1%' is not a valid type specification.") % *str).str(), context.argument_context(0));
            }
            // If the value is an instance of the type, return it
            if (validate_type(context, *type, second, context.argument_context(1))) {
                return rvalue_cast(second);
            }
        } else {
            throw evaluation_exception((boost::format("expected %1% or %2% for first argument but found %3%.") % types::type::name() % types::string::name() % first.get_type()).str(), context.argument_context(0));
        }

        // Call the lambda and give it the type of the argument
        second = second.get_type();
        return context.yield(arguments);
    }

}}}}  // namespace puppet::compiler::evaluation::functions
