#include <puppet/compiler/evaluation/operators/unary/logical_not.hpp>
#include <puppet/compiler/evaluation/operators/unary/call_context.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace unary {

    descriptor logical_not::create_descriptor()
    {
        unary::descriptor descriptor{ ast::unary_operator::logical_not };

        descriptor.add("Any", [](call_context& context) {
            return !context.operand().is_truthy();
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::unary
