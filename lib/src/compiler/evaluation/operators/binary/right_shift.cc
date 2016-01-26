#include <puppet/compiler/evaluation/operators/binary/right_shift.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    descriptor right_shift::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::right_shift };

        descriptor.add("Integer", "Integer", [](call_context& context) {
            auto left = context.left().require<int64_t>();
            auto right = context.right().require<int64_t>();

            // If right < 0. reverse direction
            // If left is negative, keep the sign bit
            if (right < 0 && left < 0) {
                return -(-left << -right);
            }
            if (right < 0) {
                return left << -right;
            }
            if (left < 0) {
                return -(-left >> right);
            }
            return left >> right;
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
