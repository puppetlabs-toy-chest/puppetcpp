#include <catch.hpp>
#include <puppet/options/commands/version.hpp>
#include <puppet/options/commands/help.hpp>
#include <puppet/options/parser.hpp>
#include <sstream>

using namespace std;
using namespace puppet;
using namespace puppet::options;

extern char const* const VERSION_COMMAND_HELP =
    "\n"
    "Usage: puppetcpp version\n"
    "\n"
    "Show the version information.\n"
    "\n"
    "This command shows the version information and exits.\n";

SCENARIO("using the version command", "[options]")
{
    ostringstream stream;

    options::parser parser;
    parser.add<commands::help>(stream);
    parser.add<commands::version>(stream);

    WHEN("given no arguments") {
        REQUIRE(parser.parse({ "version" }).execute() == EXIT_SUCCESS);
        THEN("the version should be displayed") {
            REQUIRE(stream.str() == "0.1.0\n");
        }
    }
    WHEN("given an invalid option") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "version", "--not_valid" }), option_exception);
        }
    }
    WHEN("given help is given version as a command") {
        REQUIRE(parser.parse({ "help", "version" }).execute() == EXIT_SUCCESS);
        THEN("it should display the help") {
            REQUIRE(stream.str() == VERSION_COMMAND_HELP);
        }
    }
}
