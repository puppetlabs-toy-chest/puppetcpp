#include <puppet/compiler/evaluation/operators/binary/out_edge.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    // Forward declare add_relationship (implemented in in_edge.cc)
    values::value add_relationship(call_context const& context, compiler::relationship relationship);

    descriptor out_edge::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::out_edge };

        descriptor.add("Any", "Any", [](call_context& context) {
            return add_relationship(context, relationship::require);
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
