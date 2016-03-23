#include <puppet/compiler/evaluation/operators/binary/in_edge.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    values::value add_relationship(call_context const& context, compiler::relationship relationship)
    {
        // Populate an array of resource references from the right-hand side
        values::array result;
        context.right().each_resource([&](types::resource const& target) {
            result.push_back(target);
        }, [&](string const& message) {
            throw evaluation_exception(message, context.right_context(), context.context().backtrace());
        });

        // Add the relationship; it will be evaluated upon context finalization
        context.context().add(
            resource_relationship(
                relationship,
                rvalue_cast(context.left()),
                context.left_context(),
                rvalue_cast(context.right()),
                context.right_context()
            )
        );
        return result;
    }

    descriptor in_edge::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::in_edge };

        descriptor.add("Any", "Any", [](call_context& context) {
            return add_relationship(context, relationship::before);
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
