#include <catch.hpp>
#include <puppet/options/parser.hpp>
#include <puppet/options/commands/help.hpp>
#include <sstream>

using namespace std;
using namespace puppet;
using namespace puppet::options;

extern char const* const DEFAULT_HELP;
extern char const* const HELP_COMMAND_HELP;

SCENARIO("using an options parser", "[options]")
{
    ostringstream stream;
    options::parser parser;
    parser.add<commands::help>(stream);

    WHEN("given no arguments") {
        REQUIRE(parser.parse({}).execute() == EXIT_SUCCESS);
        THEN("the default help should be displayed") {
            REQUIRE(stream.str() == DEFAULT_HELP);
        }
        WHEN("the help command is not available") {
            THEN("it should throw an exception") {
                REQUIRE_THROWS_AS(options::parser().parse({}), option_exception);
            }
        }
    }
    WHEN("given an invalid command") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({"not_a_command"}), option_exception);
        }
    }
    WHEN("given an invalid option") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({"--not_valid"}), option_exception);
        }
    }
    WHEN("given an option that matches a command") {
        REQUIRE(parser.parse({ "--help" }).execute() == EXIT_SUCCESS);
        THEN("then it should treat the option as a command") {
            REQUIRE(stream.str() == DEFAULT_HELP);
        }
    }
    WHEN("given a command") {
        REQUIRE(parser.parse({ "help", "help" }).execute() == EXIT_SUCCESS);
        THEN("then the command should be executed") {
            REQUIRE(stream.str() == HELP_COMMAND_HELP);
        }
    }
}
