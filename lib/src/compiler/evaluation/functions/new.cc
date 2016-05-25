#include <puppet/compiler/evaluation/functions/new.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    descriptor new_::create_descriptor()
    {
        functions::descriptor descriptor{ "new" };

        descriptor.add("Callable[Type, Any, 2, default, Optional[Callable[1, 1]]]", [](call_context& context) -> values::value {
            auto& type = context.argument(0).require<values::type>();

            try {
                auto value = type.instantiate(rvalue_cast(context.argument(1)), context.arguments(), 2);
                if (!context.block()) {
                    return value;
                }

                values::array arguments;
                arguments.emplace_back(rvalue_cast(value));
                return context.yield(arguments);
            } catch (values::instantiation_exception const& ex) {
                throw evaluation_exception(ex.what(), context.argument_context(0), context.context().backtrace());
            } catch (values::type_conversion_exception const& ex) {
                throw evaluation_exception(ex.what(), context.argument_context(1), context.context().backtrace());
            } catch (values::conversion_argument_exception const& ex) {
                throw evaluation_exception(ex.what(), context.argument_context(2 + ex.index()), context.context().backtrace());
            }
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
