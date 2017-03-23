#include <puppet/compiler/evaluation/functions/reduce.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <boost/optional.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    descriptor reduce::create_descriptor()
    {
        functions::descriptor descriptor{ "reduce" };

        descriptor.add("Callable[Iterable, Any, 1, 2, Callable[2, 2]]", [](call_context& context) -> values::value {
            values::array block_arguments(context.block()->parameters.size());

            boost::optional<values::value> memo;
            if (context.arguments().size() == 2) {
                memo = rvalue_cast(context.argument(1));
            }

            boost::apply_visitor(
                values::iteration_visitor{
                    [&](auto const* key, auto const& value) {
                        if (key) {
                            values::array pair(2);
                            pair[0] = *key;
                            pair[1] = value;

                            if (!memo) {
                                memo.emplace(rvalue_cast(pair));
                                return true;
                            }
                            block_arguments[1] = rvalue_cast(pair);
                        } else {
                            if (!memo) {
                                memo = value;
                                return true;
                            }
                            block_arguments[1] = value;
                        }
                        block_arguments[0] = rvalue_cast(*memo);

                        auto result = context.yield(block_arguments);
                        if (result.as<values::break_iteration>()) {
                            // Break the iteration
                            return false;
                        }
                        memo.emplace(rvalue_cast(result));
                        return true;
                    }
                },
                context.argument(0)
            );

            if (memo) {
                return rvalue_cast(*memo);
            }
            return values::undef();
        });

        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
