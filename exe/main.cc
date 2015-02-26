// The static lexer header must be included prior to including the lexer header
#include "../lexer/static_lexer.hpp"
#include "../parser/parser.hpp"
#include "../utility/error_reporter.hpp"
#include <iostream>

using namespace std;
using namespace puppet::parser;
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

    try {
        manifest = parser::parse_manifest_file(reporter, argv[1]);
    } catch (exception const& ex) {
        reporter.error("unhandled exception: %1%", ex.what());
    }

    auto errors = reporter.errors();
    auto warnings = reporter.warnings();

    cout << "parsing " << (errors > 0 ? "failed" : "succeeded") << " with " <<
            errors << " error" << (errors != 1 ? "s" : "")  <<
            " and " << warnings << " warning" << (warnings != 1 ? "s.\n" : ".\n");

    if (manifest) {
        cout << "\nparsed AST:\n" << *manifest << endl;
    }
    return errors > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
