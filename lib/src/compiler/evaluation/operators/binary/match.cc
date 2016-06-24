#include <puppet/compiler/evaluation/operators/binary/match.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    bool is_match(call_context& context, string const& left, string const& right)
    {
        try {
            return values::regex{ right }.match(context.context(), left);
        } catch (utility::regex_exception const& ex) {
            throw evaluation_exception(
                (boost::format("invalid regular expression: %1%") %
                 ex.what()
                ).str(),
                context.right_context(),
                context.context().backtrace()
            );
        }
    }

    descriptor match::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::match };

        descriptor.add("String", "String", [](call_context& context) {
            return is_match(context, context.left().require<string>(), context.right().require<string>());
        });
        descriptor.add("String", "Regexp", [](call_context& context) {
            return context.right().require<values::regex>().match(context.context(), context.left().require<string>());
        });
        descriptor.add("Any", "Type", [](call_context& context) {
            types::recursion_guard guard;
            return context.right().require<values::type>().is_instance(context.left(), guard);
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
