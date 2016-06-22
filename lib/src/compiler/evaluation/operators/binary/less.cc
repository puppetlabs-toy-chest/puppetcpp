#include <puppet/compiler/evaluation/operators/binary/less.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/unicode/string.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    descriptor less::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::less_than };

        descriptor.add("Integer", "Integer", [](call_context& context) {
            return context.left().require<int64_t>() < context.right().require<int64_t>();
        });
        descriptor.add("Integer", "Float", [](call_context& context) {
            return static_cast<double>(context.left().require<int64_t>()) < context.right().require<double>();
        });
        descriptor.add("Float", "Integer", [](call_context& context) {
            return context.left().require<double>() < static_cast<double>(context.right().require<int64_t>());
        });
        descriptor.add("Float", "Float", [](call_context& context) {
            return context.left().require<double>() < context.right().require<double>();
        });
        descriptor.add("String", "String", [](call_context& context) {
            auto& left = context.left().require<string>();
            auto& right = context.right().require<string>();
            if (left.size() < right.size()) {
                return unicode::string{ left }.compare(right, true) < 0;
            }
            return unicode::string{ right }.compare(left, true) > 0;
        });
        descriptor.add("Type", "Type", [](call_context& context) {
            auto& left = context.left().require<values::type>();
            auto& right = context.right().require<values::type>();
            types::recursion_guard guard;
            return right.is_assignable(left, guard) && left != right && !left.is_assignable(right, guard);
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
