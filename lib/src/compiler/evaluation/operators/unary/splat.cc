#include <puppet/compiler/evaluation/operators/unary/splat.hpp>
#include <puppet/compiler/evaluation/operators/unary/call_context.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace unary {

    descriptor splat::create_descriptor()
    {
        unary::descriptor descriptor{ ast::unary_operator::splat };

        descriptor.add("Any", [](call_context& context) {
            return context.operand().to_array();
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::unary
