#include <puppet/compiler/evaluation/functions/reverse_each.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    descriptor reverse_each::create_descriptor()
    {
        functions::descriptor descriptor{ "reverse_each" };

        descriptor.add("Callable[Iterable, 1, 1]", [](call_context& context) {
            return values::iterator{ rvalue_cast(context.argument(0)), 1, true };
        });

        descriptor.add("Callable[Iterable, 1, 1, Callable[1, 2]]", [](call_context& context) -> values::value {
            values::array block_arguments(context.block()->parameters.size());
            int64_t index = 0;

            boost::optional<values::value> transfer;
            values::iterator iterator{ rvalue_cast(context.argument(0)), 1, true };
            iterator.each([&](auto const* key, auto const& value) {
                if (key) {
                    if (block_arguments.size() == 1) {
                        values::array pair(2);
                        pair[0] = *key;
                        pair[1] = value;
                        block_arguments[0] = rvalue_cast(pair);
                    } else {
                        block_arguments[0] = *key;
                        block_arguments[1] = value;
                    }
                } else {
                    if (block_arguments.size() == 1) {
                        block_arguments[0] = value;
                    } else {
                        block_arguments[0] = index++;
                        block_arguments[1] = value;
                    }
                }
                auto result = context.yield(block_arguments);
                if (result.as<values::break_iteration>()) {
                    // Break the iteration
                    return false;
                }
                // Check for control transfer and break out of the loop
                if (result.is_transfer()) {
                    transfer = rvalue_cast(result);
                    return false;
                }
                return true;
            });
            if (transfer) {
                return rvalue_cast(*transfer);
            }
            return iterator;
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
