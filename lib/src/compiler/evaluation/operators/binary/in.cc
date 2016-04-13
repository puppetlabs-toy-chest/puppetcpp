#include <puppet/compiler/evaluation/operators/binary/in.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    static bool is_in(call_context& context, values::regex const& left, string const& right)
    {
        smatch matches;
        bool result = left.pattern().empty() || regex_search(right, matches, left.value());
        context.context().set(matches);
        return result;
    }

    descriptor in::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::in };

        // The order of these overloads is important (most specific to least specific)
        descriptor.add("String", "String", [](call_context& context) {
            return boost::algorithm::icontains(context.right().require<string>(), context.left().require<string>());
        });
        descriptor.add("Regexp", "String", [](call_context& context) {
            return is_in(context, context.left().require<values::regex>(), context.right().require<string>());
        });
        descriptor.add("Type", "Array[Any]", [](call_context& context) {
            auto& left = context.left().require<values::type>();
            auto& right = context.right().require<values::array>();
            types::recursion_guard guard;
            for (auto const& element : right) {
                if (left.is_instance(element, guard)) {
                    return true;
                }
            }
            return false;
        });
        descriptor.add("Regexp", "Array[Any]", [](call_context& context) {
            auto& left = context.left().require<values::regex>();
            auto& right = context.right().require<values::array>();
            for (auto const& element : right) {
                auto ptr = element->as<string>();
                if (ptr && is_in(context, left, *ptr)) {
                    return true;
                }
            }
            return false;
        });
        descriptor.add("Any", "Array[Any]", [](call_context& context) {
            auto& left = context.left();
            auto& right = context.right().require<values::array>();
            for (auto const& element : right) {
                if (left == element) {
                    return true;
                }
            }
            return false;
        });
        descriptor.add("Type", "Hash[Any, Any]", [](call_context& context) {
            auto& left = context.left().require<values::type>();
            auto& right = context.right().require<values::hash>();
            types::recursion_guard guard;
            for (auto const& kvp : right) {
                if (left.is_instance(kvp.key(), guard)) {
                    return true;
                }
            }
            return false;
        });
        descriptor.add("Any", "Hash[Any, Any]", [](call_context& context) {
            auto& left = context.left();
            auto& right = context.right().require<values::hash>();
            for (auto const& kvp : right) {
                if (left == kvp.key()) {
                    return true;
                }
            }
            return false;
        });
        descriptor.add("Any", "Any", [](call_context& context) {
            return false;
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
