#include <puppet/options/parser.hpp>
#include <puppet/options/commands/compile.hpp>
#include <puppet/options/commands/help.hpp>
#include <puppet/options/commands/parse.hpp>
#include <puppet/options/commands/repl.hpp>
#include <puppet/options/commands/version.hpp>
#include <puppet/logging/logger.hpp>

using namespace std;
using namespace puppet;
using namespace puppet::options;
using namespace puppet::logging;

int main(int argc, char const* argv[])
{
    console_logger logger;

    vector<string> arguments;
    for (int i = 1; i < argc; ++i) {
        arguments.emplace_back(argv[i]);
    }

    // The option parser should not be in the try scope because it stores commands
    options::parser parser;
    try {
        // Add the supported commands here
        parser.add<commands::compile>();
        parser.add<commands::help>();
        parser.add<commands::parse>();
        parser.add<commands::repl>();
        parser.add<commands::version>();

        return parser.parse(arguments).execute();
    } catch (option_exception const& ex) {
        LOG(error, ex.what());
        if (ex.command()) {
            LOG(notice, "use 'puppetcpp help %1%' for help.", ex.command()->name());
        } else {
            LOG(notice, "use 'puppetcpp help' for help.");
        }
        return EXIT_FAILURE;
    } catch (exception const& ex) {
        LOG(critical, "unhandled exception: %1%", ex.what());
        return EXIT_FAILURE;
    }
}
