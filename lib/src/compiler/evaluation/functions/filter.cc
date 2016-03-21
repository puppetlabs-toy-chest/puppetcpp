#include <puppet/compiler/evaluation/functions/filter.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static values::value filter_values(call_context& context, string const& argument)
    {
        values::array result;
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
            if (context.yield(block_arguments).is_true()) {
                result.emplace_back(rvalue_cast(codepoint));
            }
            return true;
        });

        return result;
    }

    static values::value filter_values(call_context& context, types::integer const& range)
    {
        if (!range.iterable()) {
            throw evaluation_exception(
                (boost::format("%1% is not an iterable range.") %
                 range
                ).str(),
                context.argument_context(0),
                context.context().backtrace()
            );
        }

        values::array result;
        values::array block_arguments(context.block()->parameters.size());

        range.each([&](int64_t index, int64_t value) {
            if (block_arguments.size() == 1) {
                block_arguments[0] = value;
            } else {
                block_arguments[0] = index;
                block_arguments[1] = value;
            }
            if (context.yield(block_arguments).is_true()) {
                result.emplace_back(value);
            }
            return true;
        });
        return result;
    }

    static values::value filter_values(call_context& context, values::array const& argument)
    {
        values::array result;
        values::array block_arguments(context.block()->parameters.size());

        for (size_t i = 0; i < argument.size(); ++i) {
            auto& element = argument[i];

            if (block_arguments.size() == 1) {
                block_arguments[0] = element;
            } else {
                block_arguments[0] = static_cast<int64_t>(i);
                block_arguments[1] = element;
            }
            if (context.yield(block_arguments).is_true()) {
                result.emplace_back(element);
            }
        }
        return result;
    }

    static values::value filter_values(call_context& context, values::hash const& argument)
    {
        values::hash result;
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
            if (context.yield(block_arguments).is_true()) {
                result.set(kvp.key(), kvp.value());
            }
        }
        return result;
    }

    descriptor filter::create_descriptor()
    {
        functions::descriptor descriptor{ "filter" };

        descriptor.add("Callable[String, 1, 1, Callable[1, 2]]", [](call_context& context) {
            return filter_values(context, context.argument(0).require<string>());
        });
        descriptor.add("Callable[Integer, 1, 1, Callable[1, 2]]", [](call_context& context) -> values::value {
            auto argument = context.argument(0).require<int64_t>();
            if (argument <= 0) {
                return values::array();
            }
            return filter_values(context, types::integer{ 0, argument });
        });
        descriptor.add("Callable[Array[Any], 1, 1, Callable[1, 2]]", [](call_context& context) {
            return filter_values(context, context.argument(0).require<values::array>());
        });
        descriptor.add("Callable[Hash[Any, Any], 1, 1, Callable[1, 2]]", [](call_context& context) {
            return filter_values(context, context.argument(0).require<values::hash>());
        });
        descriptor.add("Callable[Type[Integer], 1, 1, Callable[1, 2]]", [](call_context& context) {
            auto& argument = context.argument(0).require<values::type>();
            auto& range = boost::get<types::integer>(argument);
            return filter_values(context, range);
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
