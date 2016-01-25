#include <puppet/compiler/evaluation/operators/binary/less.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <boost/algorithm/string.hpp>

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
            return boost::ilexicographical_compare(context.left().require<string>(), context.right().require<string>());
        });
        descriptor.add("Type", "Type", [](call_context& context) {
            return context.left().require<values::type>().is_specialization(context.right().require<values::type>());
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary