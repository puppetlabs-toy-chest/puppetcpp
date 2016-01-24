#include <puppet/compiler/evaluation/operators/binary/not_match.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    // Forward declare the implementation (defined in match.cc)
    bool is_match(call_context& context, string const& left, string const& right);
    bool is_match(call_context& context, string const& left, values::regex const& right);

    descriptor not_match::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::not_match };

        descriptor.add("String", "String", [](call_context& context) {
            return !is_match(context, context.left().require<string>(), context.right().require<string>());
        });
        descriptor.add("String", "Regexp", [](call_context& context) {
            return !is_match(context, context.left().require<string>(), context.right().require<values::regex>());
        });
        descriptor.add("Any", "Type", [](call_context& context) {
            return !context.right().require<values::type>().is_instance(context.left());
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
