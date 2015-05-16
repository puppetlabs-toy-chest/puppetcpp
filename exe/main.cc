#include <puppet/parser/parser.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/logging/logger.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::parser;
using namespace puppet::runtime;
using namespace puppet::logging;
namespace ast = puppet::ast;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        cerr << "usage: puppetcpp <source_file>" << endl;
        return EXIT_FAILURE;
    }

    console_logger logger;

    ifstream file(argv[1]);
    if (!file) {
        logger.log(level::error, "could not open file '%1%'.", argv[1]);
        return EXIT_FAILURE;
    }

    try {
        auto manifest = parser::parse(file);
        logger.log(level::debug, "parsed syntax tree:\n%1%", manifest);
        if (manifest.body()) {
            // Evaluate all expressions in the manifest's body
            context ctx(logger, [&](token_position const& position, string const& message) {
                string text;
                size_t column;
                tie(text, column) = get_text_and_column(file, get<0>(position));
                logger.log(level::warning, get<1>(position), column, text, argv[1], message);
            });
            expression_evaluator evaluator(ctx);
            for (auto& expression : *manifest.body()) {
                // Top level expressions must be productive
                evaluator.evaluate(expression, true);
            }
        }
    } catch (parse_exception const& ex) {
        string text;
        size_t column;
        tie(text, column) = get_text_and_column(file, get<0>(ex.position()));
        logger.log(level::error, get<1>(ex.position()), column, text, argv[1], ex.what());
    } catch (evaluation_exception const& ex) {
        string text;
        size_t column;
        tie(text, column) = get_text_and_column(file, get<0>(ex.position()));
        logger.log(level::error, get<1>(ex.position()), column, text, argv[1], ex.what());
    } catch (exception const& ex) {
        logger.log(level::critical, "unhandled exception: %1%", ex.what());
    }

    auto errors = logger.errors();
    auto warnings = logger.warnings();

    logger.log(level::info, "compilation %1% with %2% %3% and %4% %5%.",
        (errors > 0 ? "failed" : "succeeded"),
        errors,
        (errors != 1 ? "errors" : "error"),
        warnings,
        (warnings != 1 ? "warnings" : "warning")
    );
    return errors > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
