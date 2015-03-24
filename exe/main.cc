// The static lexer header must be included prior to including the lexer header
#include <puppet/lexer/static_lexer.hpp>
#include <puppet/parser/parser.hpp>
#include <puppet/runtime/evaluator.hpp>
#include <puppet/utility/error_reporter.hpp>
#include <iostream>

using namespace std;
using namespace puppet::parser;
using namespace puppet::runtime;
using namespace puppet::utility;
namespace ast = puppet::ast;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        cerr << "usage: puppetcpp <source_file>" << endl;
        return EXIT_FAILURE;
    }

    error_reporter reporter(cerr);
    boost::optional<ast::manifest> manifest;

    std::ifstream file(argv[1]);
    if (!file) {
        reporter.error("could not open file '%1%'.", argv[1]);
        return EXIT_FAILURE;
    }

    size_t errors = 0;
    size_t warnings = 0;
    try {
        manifest = parser::parse_manifest_file(reporter, file, argv[1]);

        errors = reporter.errors();
        warnings = reporter.warnings();
        reporter.reset();

        cout << "parsing " << (errors > 0 ? "failed" : "succeeded") << " with " <<
                errors << " error" << (errors != 1 ? "s" : "") <<
                " and " << warnings << " warning" << (warnings != 1 ? "s.\n" : ".\n");

        if (manifest) {
            cout << "\nparsed AST:\n" << *manifest << endl;
            cout << "\nevaluating:\n";

            evaluator e;
            e.evaluate(reporter, *manifest, argv[1], file);

            errors = reporter.errors();
            warnings = reporter.warnings();
            reporter.reset();

            cout << "evaluation " << (errors > 0 ? "failed" : "succeeded") << " with " <<
                    errors << " error" << (errors != 1 ? "s" : "") <<
                    " and " << warnings << " warning" << (warnings != 1 ? "s.\n" : ".\n");
        }
    }
    catch (exception const& ex) {
        reporter.error("unhandled exception: %1%", ex.what());
    }
    return errors > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
