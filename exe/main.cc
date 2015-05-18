#include <puppet/compiler/node.hpp>

using namespace std;
using namespace puppet::logging;
namespace compiler = puppet::compiler;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        cerr << "usage: puppetcpp <source_file>" << endl;
        return EXIT_FAILURE;
    }

    console_logger logger;

    try {
        // Construct a dummy environment (TODO: take command line options for environment and paths)
        compiler::environment environment("production", "~/.puppetlabs/etc/code/environments/production");

        // Construct a node (TODO: pass name and facts to node)
        compiler::node node("localhost", environment);

        try {
            // Compile the manifest
            // TODO: use the default manifest file for the environment rather than specifying one on the command line
            auto catalog = node.compile(logger, argv[1]);

            // TODO: output the catalog
        } catch (compiler::compilation_exception const& ex) {
            logger.log(level::error, ex.line(), ex.column(), ex.text(), ex.path(), "node '%1%': %2%", node.name(), ex.what());
        }
    } catch (exception const& ex) {
        logger.log(level::critical, "unhandled exception: %1%", ex.what());
    }

    auto errors = logger.errors();
    auto warnings = logger.warnings();

    logger.log(level::notice, "compilation %1% with %2% %3% and %4% %5%.",
        (errors > 0 ? "failed" : "succeeded"),
        errors,
        (errors != 1 ? "errors" : "error"),
        warnings,
        (warnings != 1 ? "warnings" : "warning")
    );
    return errors > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
