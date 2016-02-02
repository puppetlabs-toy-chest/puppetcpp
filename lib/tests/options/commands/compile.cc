#include <catch.hpp>
#include <puppet/options/commands/compile.hpp>
#include <puppet/options/commands/help.hpp>
#include <puppet/options/parser.hpp>
#include <sstream>

using namespace std;
using namespace puppet;
using namespace puppet::options;

extern char const* const COMPILE_COMMAND_HELP =
    "\n"
    "Usage: puppetcpp compile [options] [[manifest] [manifest] ...]\n"
    "\n"
    "Compile Puppet manifests into a Puppet catalog.\n"
    "\n"
    "Options:\n"
    "\n"
    "  --code-dir arg                        The Puppet code directory to use. \n"
    "                                        Defaults to the current platform's code\n"
    "                                        directory.\n"
    "  --color                               Forces color output on platforms that \n"
    "                                        support colorized output.\n"
    "  -d [ --debug ]                        Enable debug output.\n"
    "  -e [ --environment ] arg (=production)\n"
    "                                        The environment to compile for.\n"
    "  --environment-dir arg                 Specifies a directory to search for \n"
    "                                        environments.\n"
    "  -f [ --facts ] arg                    The path to the YAML facts file to use.\n"
    "                                        Defaults to the current system's facts.\n"
    "  -g [ --graph ] arg                    The path to write a DOT language file \n"
    "                                        for viewing the catalog dependency \n"
    "                                        graph.\n"
    "  --help                                Display command help.\n"
    "  -l [ --log-level ] arg (=notice)      Set logging level.\n"
    "                                        Supported levels: debug, info, notice, \n"
    "                                        warning, error, alert, emergency, \n"
    "                                        critical.\n"
    "  --module-dir arg                      Specifies a directory to search for \n"
    "                                        global modules.\n"
    "  -n [ --node ] arg                     The node name to use. Defaults to the \n"
    "                                        'fqdn' fact.\n"
    "  --no-color                            Disables color output.\n"
    "  -o [ --output ] arg (=catalog.json)   The output path for the compiled \n"
    "                                        catalog.\n"
    "  --verbose                             Enable verbose (info) output.\n"
    "\n"
    "Compiles a Puppet manifest into a Puppet catalog. When invoked with no options,\n"
    "the compiler will compile the manifest for the 'production' environment.\n"
    "\n"
    "Manifests will be evaluated in the order they are presented on the command line.\n";

SCENARIO("using the compile command", "[options]")
{
    ostringstream stream;

    options::parser parser;
    parser.add<commands::help>(stream);
    parser.add<commands::compile>();

    WHEN("given an invalid option") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "compile", "--not_valid" }), option_exception);
        }
    }
    WHEN("given help is given compile as a command") {
        REQUIRE(parser.parse({ "help", "compile" }).execute() == EXIT_SUCCESS);
        THEN("it should display the help") {
            REQUIRE(stream.str() == COMPILE_COMMAND_HELP);
        }
    }
    WHEN("given conflicting logging options") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "compile", "--debug", "--verbose" }), option_exception);
            REQUIRE_THROWS_AS(parser.parse({ "compile", "--debug", "-lverbose" }), option_exception);
            REQUIRE_THROWS_AS(parser.parse({ "compile", "--verbose", "--loglevel=debug" }), option_exception);
        }
    }
    WHEN("given conflicting colorization options") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "compile", "--color", "--no-color" }), option_exception);
        }
    }
    WHEN("given a code directory that does not exist") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "compile", "--code-dir", "does_not_exist" }), option_exception);
        }
    }
    WHEN("given an environment directory that does not exist") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "compile", "--environment-dir", "does_not_exist" }), option_exception);
        }
    }
}
