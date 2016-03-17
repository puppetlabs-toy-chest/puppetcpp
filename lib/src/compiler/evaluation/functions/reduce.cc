#include <puppet/compiler/evaluation/functions/reduce.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static values::value reduce_values(call_context& context, boost::optional<values::value> memo, string const& argument)
    {
        values::array block_arguments(2);

        // Enumerate the string as Unicode codepoints
        values::enumerate_string(argument, [&](string codepoint) {
            // If no memo, set it now
            if (!memo) {
                memo.emplace(rvalue_cast(codepoint));
                return true;
            }

            block_arguments[0] = rvalue_cast(*memo);
            block_arguments[1] = rvalue_cast(codepoint);
            memo.emplace(context.yield(block_arguments));
            return true;
        });

        if (memo) {
            return rvalue_cast(*memo);
        }
        return values::undef();
    }

    static values::value reduce_values(call_context& context, boost::optional<values::value> memo, types::integer const& range)
    {
        if (!range.enumerable()) {
            throw evaluation_exception((boost::format("%1% is not enumerable.") % range).str(), context.argument_context(0));
        }

        values::array block_arguments(2);

        range.each([&](int64_t index, int64_t value) {
            // If no memo, set it now
            if (!memo) {
                memo = value;
                return true;
            }

            block_arguments[0] = rvalue_cast(*memo);
            block_arguments[1] = value;
            memo.emplace(context.yield(block_arguments));
            return true;
        });

        if (memo) {
            return rvalue_cast(*memo);
        }
        return values::undef();
    }

    static values::value reduce_values(call_context& context, boost::optional<values::value> memo, values::array const& argument)
    {
        values::array block_arguments(2);

        for (size_t i = 0; i < argument.size(); ++i) {
            auto& element = argument[i];

            // If no memo, set it now
            if (!memo) {
                memo = element;
                continue;
            }

            block_arguments[0] = rvalue_cast(*memo);
            block_arguments[1] = element;
            memo.emplace(context.yield(block_arguments));
        }

        if (memo) {
            return rvalue_cast(*memo);
        }
        return values::undef();
    }

    static values::value reduce_values(call_context& context, boost::optional<values::value> memo, values::hash const& argument)
    {
        values::array block_arguments(2);

        for (auto& kvp : argument) {
            values::array pair(2);
            pair[0] = kvp.key();
            pair[1] = kvp.value();

            // If no memo, set it now
            if (!memo) {
                memo.emplace(rvalue_cast(pair));
                continue;
            }

            block_arguments[0] = rvalue_cast(*memo);
            block_arguments[1] = rvalue_cast(pair);
            memo.emplace(context.yield(block_arguments));
        }

        if (memo) {
            return rvalue_cast(*memo);
        }
        return values::undef();
    }

    descriptor reduce::create_descriptor()
    {
        functions::descriptor descriptor{ "reduce" };

        // Utility function for extracting the memo
        auto static const extract_memo = [](call_context& context) -> boost::optional<values::value> {
            if (context.arguments().size() < 2) {
                return boost::none;
            }
            return rvalue_cast(context.argument(1));
        };

        descriptor.add("Callable[String, Any, 1, 2, Callable[2, 2]]", [&](call_context& context) {
            return reduce_values(context, extract_memo(context), context.argument(0).require<string>());
        });
        descriptor.add("Callable[Integer, Any, 1, 2, Callable[2, 2]]", [&](call_context& context) -> values::value {
            auto argument = context.argument(0).require<int64_t>();
            if (argument <= 0) {
                return values::undef();
            }
            return reduce_values(context, extract_memo(context), types::integer{ 0, argument - 1 });
        });
        descriptor.add("Callable[Array[Any], Any, 1, 2, Callable[2, 2]]", [&](call_context& context) {
            return reduce_values(context, extract_memo(context), context.argument(0).require<values::array>());
        });
        descriptor.add("Callable[Hash[Any, Any], Any, 1, 2, Callable[2, 2]]", [&](call_context& context) {
            return reduce_values(context, extract_memo(context), context.argument(0).require<values::hash>());
        });
        descriptor.add("Callable[Type[Integer], Any, 1, 2, Callable[2, 2]]", [&](call_context& context) {
            auto& argument = context.argument(0).require<values::type>();
            auto& range = boost::get<types::integer>(argument);
            return reduce_values(context, extract_memo(context), range);
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
