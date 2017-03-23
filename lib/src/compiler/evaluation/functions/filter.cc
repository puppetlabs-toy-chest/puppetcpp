#include <puppet/compiler/evaluation/functions/filter.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static values::value iterate_hash(call_context& context)
    {
        values::array block_arguments(context.block()->parameters.size());
        values::hash result;

        boost::optional<values::value> transfer;
        boost::apply_visitor(
            values::iteration_visitor{
                [&](auto const* key, auto const& value) {
                    if (!key) {
                        throw runtime_error("expected a key.");
                    }
                    if (block_arguments.size() == 1) {
                        values::array pair(2);
                        pair[0] = *key;
                        pair[1] = value;
                        block_arguments[0] = rvalue_cast(pair);
                    } else {
                        block_arguments[0] = *key;
                        block_arguments[1] = value;
                    }
                    auto filtered = context.yield(block_arguments);
                    if (filtered.as<values::break_iteration>()) {
                        // Break the iteration
                        return false;
                    }
                    // Check for control transfer and break out of the loop
                    if (filtered.is_transfer()) {
                        transfer = filtered;
                        return false;
                    }
                    if (filtered.is_true()) {
                        result.set(*key, value);
                    }
                    return true;
                }
            },
            context.argument(0)
        );
        if (transfer) {
            return rvalue_cast(*transfer);
        }
        return result;
    }

    descriptor filter::create_descriptor()
    {
        functions::descriptor descriptor{ "filter" };

        descriptor.add("Callable[Iterable, 1, 1, Callable[1, 2]]", [](call_context& context) -> values::value {

            // If a hash or iterating a hash, return a hash
            if (context.argument(0).as<values::hash>()) {
                return iterate_hash(context);
            }
            if (auto iterator = context.argument(0).as<values::iterator>()) {
                if (iterator->value().as<values::hash>()) {
                    return iterate_hash(context);
                }
            }

            values::array block_arguments(context.block()->parameters.size());
            int64_t index = 0;
            values::array result;

            boost::optional<values::value> transfer;
            boost::apply_visitor(
                values::iteration_visitor{
                    [&](auto const* key, auto const& value) {
                        if (key) {
                            throw runtime_error("expected a null key.");
                        }
                        if (block_arguments.size() == 1) {
                            block_arguments[0] = value;
                        } else {
                            block_arguments[0] = index++;
                            block_arguments[1] = value;
                        }
                        auto filtered = context.yield(block_arguments);
                        if (filtered.as<values::break_iteration>()) {
                            // Break the iteration
                            return false;
                        }
                        // Check for control transfer and break out of the loop
                        if (filtered.is_transfer()) {
                            transfer = filtered;
                            return false;
                        }
                        if (filtered.is_true()) {
                            result.emplace_back(value);
                        }
                        return true;
                    }
                },
                context.argument(0)
            );
            if (transfer) {
                return rvalue_cast(*transfer);
            }
            return result;
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
