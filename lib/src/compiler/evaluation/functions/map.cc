#include <puppet/compiler/evaluation/functions/map.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static values::value map_values(call_context& context, string const& argument)
    {
        values::array result;
        result.reserve(argument.size());

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
            result.emplace_back(context.yield(block_arguments));
            return true;
        });

        return result;
    }

    static values::value map_values(call_context& context, types::integer const& range)
    {
        if (!range.enumerable()) {
            throw evaluation_exception(
                (boost::format("%1% is not enumerable.") %
                 range
                ).str(),
                context.argument_context(0),
                context.context().backtrace()
            );
        }

        values::array result;
        result.reserve(range.size());

        values::array block_arguments(context.block()->parameters.size());

        range.each([&](int64_t index, int64_t value) {
            if (block_arguments.size() == 1) {
                block_arguments[0] = value;
            } else {
                block_arguments[0] = index;
                block_arguments[1] = value;
            }
            result.emplace_back(context.yield(block_arguments));
            return true;
        });
        return result;
    }

    static values::value map_values(call_context& context, values::array const& argument)
    {
        values::array result;
        result.reserve(argument.size());

        values::array block_arguments(context.block()->parameters.size());

        for (size_t i = 0; i < argument.size(); ++i) {
            auto& element = argument[i];

            if (block_arguments.size() == 1) {
                block_arguments[0] = element;
            } else {
                block_arguments[0] = static_cast<int64_t>(i);
                block_arguments[1] = element;
            }
            result.emplace_back(context.yield(block_arguments));
        }
        return result;
    }

    static values::value map_values(call_context& context, values::hash const& argument)
    {
        values::array result;
        result.reserve(argument.size());

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
            result.emplace_back(context.yield(block_arguments));
        }
        return result;
    }

    descriptor map::create_descriptor()
    {
        functions::descriptor descriptor{ "map" };

        descriptor.add("Callable[String, 1, 1, Callable[1, 2]]", [](call_context& context) {
            return map_values(context, context.argument(0).require<string>());
        });
        descriptor.add("Callable[Integer, 1, 1, Callable[1, 2]]", [](call_context& context) -> values::value {
            auto argument = context.argument(0).require<int64_t>();
            if (argument <= 0) {
                return values::array();
            }
            return map_values(context, types::integer{ 0, argument - 1 });
        });
        descriptor.add("Callable[Array[Any], 1, 1, Callable[1, 2]]", [](call_context& context) {
            return map_values(context, context.argument(0).require<values::array>());
        });
        descriptor.add("Callable[Hash[Any, Any], 1, 1, Callable[1, 2]]", [](call_context& context) {
            return map_values(context, context.argument(0).require<values::hash>());
        });
        descriptor.add("Callable[Type[Integer], 1, 1, Callable[1, 2]]", [](call_context& context) {
            auto& argument = context.argument(0).require<values::type>();
            auto& range = boost::get<types::integer>(argument);
            return map_values(context, range);
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
