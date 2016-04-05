#include <puppet/compiler/evaluation/functions/assert_type.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static values::value assert_type_is(call_context& context, values::type const& type)
    {
        auto& instance = context.argument(1);
        types::recursion_guard guard;
        if (type.is_instance(instance, guard)) {
            return rvalue_cast(instance);
        }

        // Otherwise, a block is required
        if (!context.block()) {
            throw evaluation_exception(
                (boost::format("type assertion failure: expected %1% but found %2%.") %
                 type %
                 instance.get_type()
                ).str(),
                context.argument_context(1),
                context.context().backtrace()
            );
        }

        // Call the block and give it the type of the argument as the second argument
        instance = instance.get_type();
        return context.yield(context.arguments());
    }

    descriptor assert_type::create_descriptor()
    {
        functions::descriptor descriptor{ "assert_type" };

        descriptor.add("Callable[Type, Any, 2, 2, Optional[Callable[2, 2]]]", [](call_context& context) {
            return assert_type_is(context, context.argument(0).require<values::type>());
        });
        descriptor.add("Callable[String, Any, 2, 2, Optional[Callable[2, 2]]]", [](call_context& context) {
            auto& type_string = context.argument(0).require<string>();

            boost::optional<values::type> type;
            try {
                type = values::type::parse(type_string, &context.context());
            } catch (evaluation_exception const& ex) {
                throw evaluation_exception(
                    (boost::format("the expression '%1%' is not a valid type specification: %2%") %
                     type_string %
                     ex.what()
                    ).str(),
                    context.argument_context(0),
                    ex.backtrace()
                );
            }

            if (!type) {
                throw evaluation_exception(
                    (boost::format("the expression '%1%' is not a valid type specification.") %
                     type_string
                    ).str(),
                    context.argument_context(0),
                    context.context().backtrace()
                );
            }
            return assert_type_is(context, *type);
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
