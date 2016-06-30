#include <puppet/compiler/evaluation/functions/epp.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static values::value evaluate_epp(call_context& context)
    {
        auto& node = context.context().node();
        auto& logger = node.logger();
        auto& environment = node.environment();

        auto& subpath = context.argument(0).require<string>();
        auto path = environment.resolve_path(logger, find_type::template_, subpath);
        if (path.empty()) {
            throw evaluation_exception(
                (boost::format("could not find template file '%1%'.") %
                 subpath
                ).str(),
                context.argument_context(0),
                context.context().backtrace()
            );
        }

        values::hash arguments;
        if (context.arguments().size() > 1) {
            arguments = context.argument(1).move_as<values::hash>();
        }

        try {
            // Parse the EPP template
            auto tree = parser::parse_file(logger, path, nullptr, true);
            LOG(debug, "parsed EPP AST:\n-----\n%1%\n-----", *tree);

            // Validate as EPP
            tree->validate(true);

            // Create a local output stream
            ostringstream os;
            scoped_output_stream epp_stream{ context.context(), os };

            // Evaluate the syntax tree
            evaluation::evaluator evaluator{ context.context() };
            evaluator.evaluate(*tree, &arguments);
            return os.str();
        } catch (parse_exception const& ex) {
            // Log the underlying problem and then throw an error pointing at the argument
            ifstream input{ path };
            auto info = lexer::get_line_info(input, ex.begin().offset(), ex.end().offset() - ex.begin().offset());
            logger.log(logging::level::error, ex.begin().line(), info.column, info.length, info.text, path, ex.what());
            throw evaluation_exception(
                "parsing of EPP template failed.",
                context.argument_context(0),
                context.context().backtrace()
            );
        } catch (evaluation_exception const& ex) {
            ifstream input{ path };
            auto& c = ex.context();
            auto info = lexer::get_line_info(input, c.begin.offset(), c.end.offset() - c.begin.offset());
            logger.log(logging::level::error, c.begin.line(), info.column, info.length, info.text, path, ex.what());
            throw evaluation_exception(
                "evaluation of EPP template failed.",
                context.name(),
                ex.backtrace()
            );
        } catch (argument_exception const& ex) {
            throw evaluation_exception(
                (boost::format("EPP template argument error: %1%") %
                 ex.what()
                ).str(),
                context.argument_context(1),
                context.context().backtrace()
            );
        }
    }

    descriptor epp::create_descriptor()
    {
        functions::descriptor descriptor{ "epp" };
        descriptor.add("Callable[String, Hash[String, Any], 1, 2]", evaluate_epp);
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
