#include <puppet/compiler/evaluation/functions/each.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static void each_value(call_context& context, string const& argument)
    {
        values::array block_arguments(context.block()->parameters.size());

        // Enumerate the string as Unicode codepoints
        int64_t i = 0;
        values::enumerate_string(argument, [&](string codepoint) {
            if (block_arguments.size() == 1) {
                block_arguments[0] = codepoint;
            } else {
                block_arguments[0] = i++;
                block_arguments[1] = codepoint;
            }
            context.yield(block_arguments);
            return true;
        });
    }

    static void each_value(call_context& context, types::integer const& range)
    {
        if (!range.enumerable()) {
            throw evaluation_exception((boost::format("%1% is not enumerable.") % range).str(), context.argument_context(0));
        }

        values::array block_arguments(context.block()->parameters.size());

        range.each([&](int64_t index, int64_t value) {
            if (block_arguments.size() == 1) {
                block_arguments[0] = value;
            } else {
                block_arguments[0] = index;
                block_arguments[1] = value;
            }
            context.yield(block_arguments);
            return true;
        });
    }

    static void each_value(call_context& context, values::array const& argument)
    {
        values::array block_arguments(context.block()->parameters.size());

        for (size_t i = 0; i < argument.size(); ++i) {
            auto& element = argument[i];

            if (block_arguments.size() == 1) {
                block_arguments[0] = element;
            } else {
                block_arguments[0] = static_cast<int64_t>(i);
                block_arguments[1] = element;
            }
            context.yield(block_arguments);
        }
    }

    static void each_value(call_context& context, values::hash const& argument)
    {
        values::array block_arguments(context.block()->parameters.size());

        for (auto& kvp : argument) {
            if (block_arguments.size() == 1) {
                values::array pair(2);
                pair[0] = kvp.key();
                pair[1] = kvp.value();
                block_arguments[0] = rvalue_cast(pair);
            } else {
                block_arguments[0] = kvp.key();
                block_arguments[1] = kvp.value();
            }
            context.yield(block_arguments);
        }
    }

    descriptor each::create_descriptor()
    {
        functions::descriptor descriptor{ "each" };

        descriptor.add("Callable[String, 1, 1, Callable[1, 2]]", [](call_context& context) {
            each_value(context, context.argument(0).require<string>());
            return rvalue_cast(context.argument(0));
        });
        descriptor.add("Callable[Integer, 1, 1, Callable[1, 2]]", [](call_context& context) -> values::value {
            auto argument = context.argument(0).require<int64_t>();
            if (argument > 0) {
                each_value(context, types::integer{0, argument - 1});
            }
            return rvalue_cast(context.argument(0));
        });
        descriptor.add("Callable[Array[Any], 1, 1, Callable[1, 2]]", [](call_context& context) {
            each_value(context, context.argument(0).require<values::array>());
            return rvalue_cast(context.argument(0));
        });
        descriptor.add("Callable[Hash[Any, Any], 1, 1, Callable[1, 2]]", [](call_context& context) {
            each_value(context, context.argument(0).require<values::hash>());
            return rvalue_cast(context.argument(0));
        });
        descriptor.add("Callable[Type[Integer], 1, 1, Callable[1, 2]]", [](call_context& context) {
            each_value(context, boost::get<types::integer>(context.argument(0).require<values::type>()));
            return rvalue_cast(context.argument(0));
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
