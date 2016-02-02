#include <catch.hpp>
#include <puppet/options/commands/help.hpp>
#include <puppet/options/commands/version.hpp>
#include <puppet/options/parser.hpp>
#include <sstream>

using namespace std;
using namespace puppet;
using namespace puppet::options;

extern char const* const VERSION_COMMAND_HELP;

extern char const* const DEFAULT_HELP =
    "\n"
    "Usage: puppetcpp <command> [options]\n"
    "\n"
    "Compiles Puppet manifests into Puppet catalogs.\n"
    "\n"
    "Commands:\n"
    "\n"
    "  help  Display help information.\n"
    "\n"
    "Run 'puppetcpp help <command>' for more information on a command.\n";

extern char const* const HELP_COMMAND_HELP =
    "\n"
    "Usage: puppetcpp help [command]\n"
    "\n"
    "Display help information.\n"
    "\n"
    "If a command is given, the help for that command will be displayed.\n";

SCENARIO("using the help command", "[options]")
{
    ostringstream stream;

    options::parser parser;
    parser.add<commands::help>(stream);

    WHEN("given no arguments") {
        REQUIRE(parser.parse({ "help" }).execute() == EXIT_SUCCESS);
        THEN("the default help should be displayed") {
            REQUIRE(stream.str() == DEFAULT_HELP);
        }
    }
    WHEN("given an invalid option") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "help", "--not_valid" }), option_exception);
        }
    }
    WHEN("given a command") {
        parser.add<commands::version>();
        REQUIRE(parser.parse({ "help", "version" }).execute() == EXIT_SUCCESS);
        THEN("it should display the command's help") {
            REQUIRE(stream.str() == VERSION_COMMAND_HELP);
        }
    }
}
