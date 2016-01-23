#include <puppet/compiler/evaluation/operators/binary/greater_equal.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    descriptor greater_equal::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::greater_equals };

        descriptor.add("Integer", "Integer", [](call_context& context) {
            return context.left().require<int64_t>() >= context.right().require<int64_t>();
        });
        descriptor.add("Integer", "Float", [](call_context& context) {
            return static_cast<double>(context.left().require<int64_t>()) >= context.right().require<double>();
        });
        descriptor.add("Float", "Integer", [](call_context& context) {
            return context.left().require<double>() >= static_cast<double>(context.right().require<int64_t>());
        });
        descriptor.add("Float", "Float", [](call_context& context) {
            return context.left().require<double>() >= context.right().require<double>();
        });
        descriptor.add("String", "String", [](call_context& context) {
            auto& left = context.left().require<string>();
            auto& right = context.right().require<string>();
            return !boost::ilexicographical_compare(left, right);
        });
        descriptor.add("Type", "Type", [](call_context& context) {
            auto& left = context.left().require<values::type>();
            auto& right = context.right().require<values::type>();
            return left == right || right.is_specialization(left);
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
