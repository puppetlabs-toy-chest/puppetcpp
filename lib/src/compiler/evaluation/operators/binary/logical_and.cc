#include <puppet/compiler/evaluation/operators/binary/logical_and.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    descriptor logical_and::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::logical_and };

        descriptor.add("Any", "Any", [](call_context& context) {
            return context.left().is_truthy() && context.right().is_truthy();
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
