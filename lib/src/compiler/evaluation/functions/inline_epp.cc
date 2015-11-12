#include <puppet/compiler/evaluation/functions/inline_epp.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    value inline_epp::operator()(function_call_context& context) const
    {
        // Check the argument count
        auto& arguments = context.arguments();
        auto count = arguments.size();
        if (count == 0 || count > 2) {
            throw evaluation_exception((boost::format("expected 1 or 2 arguments to '%1%' function but %2% were given.") % context.name() % count).str(), count > 2 ? context.argument_context(2) : context.call_site());
        }
        // First argument should be a string
        auto input = arguments[0]->as<string>();
        if (!input) {
            throw evaluation_exception((boost::format("expected %1% for first argument but found %2%.") % types::string::name() % arguments[0]->get_type()).str(), context.argument_context(0));
        }
        // Verify the template arguments if present
        values::hash template_arguments;
        if (count > 1) {
            if (!arguments[1]->as<values::hash>()) {
                throw evaluation_exception((boost::format("expected %1% for second argument but found %2%.") % types::hash::name() % arguments[1]->get_type()).str(), context.argument_context(1));
            }

            template_arguments = arguments[1]->move_as<values::hash>();

            // Ensure all keys are strings
            for (auto const& kvp : template_arguments) {
                if (!kvp.key().as<string>()) {
                    throw evaluation_exception((boost::format("expected all keys in EPP template arguments to be %1% found %2%.") % types::string::name() % kvp.key().get_type()).str(), context.argument_context(1));
                }
            }
        }

        auto& evaluation_context = context.context();
        auto& logger = evaluation_context.node().logger();
        static auto path = "<epp>";

        try {
            // Parse the string as an EPP template
            auto tree = parser::parse_string(*input, path, nullptr, true);
            LOG(debug, "parsed inline EPP AST:\n-----\n%1%\n-----", *tree);

            // Create a local EPP stream
            ostringstream os;
            local_epp_stream epp_stream{ evaluation_context, os };

            // Evaluate the syntax tree
            evaluation::evaluator evaluator{ evaluation_context };
            evaluator.evaluate(*tree, &template_arguments);
            return os.str();
        } catch (parse_exception const& ex) {
            // Log the underlying problem and then throw an error pointing at the argument
            size_t column;
            string text;
            tie(text, column) = lexer::get_text_and_column(*input, ex.position().offset());
            evaluation_context.node().logger().log(logging::level::error, ex.position().line(), column, text, path, ex.what());
            throw evaluation_exception("parsing of EPP template failed.", context.argument_context(0));
        } catch (argument_exception const& ex) {
            throw evaluation_exception((boost::format("EPP template argument error: %1%") % ex.what()).str(), context.argument_context(1));
        } catch (evaluation_exception const& ex) {
            // Log the underlying problem and then throw an error pointing at the argument
            evaluation_context.log(logging::level::error, ex.what(), &ex.context());
            throw evaluation_exception("evaluation of EPP template failed.", context.argument_context(0));
        }
    }

}}}}  // namespace puppet::compiler::evaluation::functions
