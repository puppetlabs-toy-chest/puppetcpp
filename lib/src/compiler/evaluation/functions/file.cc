#include <puppet/compiler/evaluation/functions/file.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <sstream>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    descriptor file::create_descriptor()
    {
        functions::descriptor descriptor{ "file" };

        descriptor.add("Callable[String, 1]", [](call_context& context) {
            auto& node = context.context().node();
            auto& environment = node.environment();

            for (size_t i = 0; i < context.arguments().size(); ++i) {
                auto& argument = *context.arguments()[i];
                auto path = environment.resolve_path(node.logger(), find_type::file, argument.require<string>());
                if (path.empty()) {
                    continue;
                }
                ifstream stream{ path };
                if (!stream) {
                    throw evaluation_exception(
                        (boost::format("could not open file '%1%' for reading.") %
                         path
                        ).str(),
                        context.argument_context(i),
                        context.context().backtrace()
                    );
                }
                ostringstream output;
                output << stream.rdbuf();
                return output.str();
            }
            throw evaluation_exception(
                "could not find any of the specified files.",
                context.name(),
                context.context().backtrace()
            );
        });
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
