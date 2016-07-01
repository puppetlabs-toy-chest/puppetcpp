#include <puppet/compiler/evaluation/functions/inline_epp.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    static values::value evaluate_epp(call_context& context)
    {
        auto& input = context.argument(0).require<std::string>();
        values::hash arguments;

        if (context.arguments().size() > 1) {
            arguments = context.argument(1).move_as<values::hash>();
        }

        auto& evaluation_context = context.context();
        auto& logger = evaluation_context.node().logger();
        static auto path = "<epp>";

        try {
            // Parse the string as an EPP template
            auto tree = parser::parse_string(logger, input, path, nullptr, true);
            LOG(debug, "parsed inline EPP AST:\n-----\n%1%\n-----", *tree);

            // Validate as EPP
            tree->validate(true);

            // Create a local output stream
            ostringstream os;
            scoped_output_stream epp_stream{ evaluation_context, os };

            // Evaluate the syntax tree
            evaluation::evaluator evaluator{ evaluation_context };
            evaluator.evaluate(*tree, &arguments);
            return os.str();
        } catch (parse_exception const& ex) {
            // Log the underlying problem and then throw an error pointing at the argument
            auto info = lexer::get_line_info(input, ex.begin().offset(), ex.end().offset() - ex.begin().offset());
            evaluation_context.node().logger().log(logging::level::error, ex.begin().line(), info.column, info.length, info.text, path, ex.what());
            throw evaluation_exception(
                "parsing of inline EPP template failed.",
                context.argument_context(0),
                evaluation_context.backtrace()
            );
        } catch (evaluation_exception const& ex) {
            auto& c = ex.context();
            auto info = lexer::get_line_info(input, c.begin.offset(), c.end.offset() - c.begin.offset());
            logger.log(logging::level::error, c.begin.line(), info.column, info.length, info.text, path, ex.what());
            throw evaluation_exception(
                "evaluation of inline EPP template failed.",
                context.name(),
                ex.backtrace()
            );
        } catch (argument_exception const& ex) {
            throw evaluation_exception(
                (boost::format("EPP template argument error: %1%") %
                 ex.what()
                ).str(),
                context.argument_context(1),
                evaluation_context.backtrace()
            );
        }
    }

    descriptor inline_epp::create_descriptor()
    {
        functions::descriptor descriptor{ "inline_epp" };
        descriptor.add("Callable[String, Hash[String, Any], 1, 2]", evaluate_epp);
        return descriptor;
    }

}}}}  // namespace puppet::compiler::evaluation::functions
