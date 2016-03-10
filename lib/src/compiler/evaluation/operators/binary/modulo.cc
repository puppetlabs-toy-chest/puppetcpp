#include <puppet/compiler/evaluation/operators/binary/modulo.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    descriptor modulo::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::modulo };

        descriptor.add("Integer", "Integer", [](call_context& context) {
            auto left = context.left().require<int64_t>();
            auto right = context.right().require<int64_t>();
            if (right == 0) {
                throw evaluation_exception("cannot divide by zero.", context.right_context(), context.context().backtrace());
            }
            return left % right;
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
