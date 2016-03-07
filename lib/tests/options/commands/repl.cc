#include <catch.hpp>
#include <puppet/options/commands/repl.hpp>
#include <puppet/options/commands/help.hpp>
#include <puppet/options/parser.hpp>
#include <sstream>

using namespace std;
using namespace puppet;
using namespace puppet::options;

extern char const* const REPL_COMMAND_HELP =
    "\n"
    "Usage: puppetcpp repl [options]\n"
    "\n"
    "Runs an interactive Puppet shell.\n"
    "\n"
    "Options:\n"
    "\n"
    "  --code-dir arg                        The Puppet code directory to use. \n"
    "                                        Defaults to the current platform's code\n"
    "                                        directory.\n"
    "  --color                               Force color output on platforms that \n"
    "                                        support colorized output.\n"
    "  -d [ --debug ]                        Enable debug output.\n"
    "  -e [ --environment ] arg (=production)\n"
    "                                        The environment to use.\n"
    "  --environment-path arg                The list of paths to use for finding \n"
    "                                        environments.\n"
    "  -f [ --facts ] arg                    The path to the YAML facts file to use.\n"
    "                                        Defaults to the current system's facts.\n"
    "  -g [ --graph-file ] arg               The path to write a DOT language file \n"
    "                                        for viewing the catalog dependency \n"
    "                                        graph.\n"
    "  --help                                Display command help.\n"
    "  -l [ --log-level ] arg (=notice)      Set logging level.\n"
    "                                        Supported levels: debug, info, notice, \n"
    "                                        warning, error, alert, emergency, \n"
    "                                        critical.\n"
    "  --module-path arg                     The list of paths to use for finding \n"
    "                                        modules.\n"
    "  -n [ --node ] arg                     The node name to use. Defaults to the \n"
    "                                        'fqdn' fact.\n"
    "  --no-color                            Disable color output.\n"
    "  -o [ --output ] arg                   The output path for the compiled \n"
    "                                        catalog.\n"
    "  --verbose                             Enable verbose output (info level).\n"
    "\n"
    "Runs the read-evel-print-loop (REPL) shell for the Puppet language. The shell is\n"
    "capable of interactively evaluating Puppet code as if being evaluated from a\n"
    "manifest file.\n"
    "\n"
    "The REPL shell incrementally builds a resource catalog that can optionally be\n"
    "output after the shell is exited.\n"
    "\n"
    "To exit the shell, type 'exit' and hit <ENTER>.\n"
    ;

SCENARIO("using the repl command", "[options]")
{
    ostringstream stream;

    options::parser parser;
    parser.add<commands::help>(stream);
    parser.add<commands::repl>();

    WHEN("given an invalid option") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "repl", "--not_valid" }), option_exception);
        }
    }
    WHEN("repl is passed to the help commannd") {
        REQUIRE(parser.parse({ "help", "repl" }).execute() == EXIT_SUCCESS);
        THEN("it should display the help") {
            REQUIRE(stream.str() == REPL_COMMAND_HELP);
        }
    }
    WHEN("given conflicting logging options") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "repl", "--debug", "--verbose" }), option_exception);
            REQUIRE_THROWS_AS(parser.parse({ "repl", "--debug", "-lverbose" }), option_exception);
            REQUIRE_THROWS_AS(parser.parse({ "repl", "--verbose", "--loglevel=debug" }), option_exception);
        }
    }
    WHEN("given an invalid log level") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "repl", "--loglevel=notvalid" }), option_exception);
        }
    }
    WHEN("given conflicting colorization options") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "repl", "--color", "--no-color" }), option_exception);
        }
    }
    WHEN("given a code directory that does not exist") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "repl", "--code-dir", "does_not_exist" }), option_exception);
        }
    }
    WHEN("given an environment directory that does not exist") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "repl", "--environment-dir", "does_not_exist" }), option_exception);
        }
    }
}
