#include <catch.hpp>
#include <puppet/options/commands/parse.hpp>
#include <puppet/options/commands/help.hpp>
#include <puppet/options/parser.hpp>
#include <sstream>

using namespace std;
using namespace puppet;
using namespace puppet::options;

extern char const* const PARSE_COMMAND_HELP =
    "\n"
    "Usage: puppetcpp parse [options] [[manifest | directory] ...]\n"
    "\n"
    "Parse Puppet manifests into intermediate representations (XPP).\n"
    "\n"
    "Options:\n"
    "\n"
    "  --as-module                           Parse directories as Puppet modules.\n"
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
    "  --help                                Display command help.\n"
    "  -l [ --log-level ] arg (=warning)     Set logging level.\n"
    "                                        Supported levels: debug, info, warning,\n"
    "                                        error.\n"
    "  --module-path arg                     The list of paths to use for finding \n"
    "                                        modules.\n"
    "  --no-color                            Disable color output.\n"
    "  -o [ --output ] arg                   The output path when parsing a single \n"
    "                                        input manifest.\n"
    "  --output-subdir arg                   The output subdirectory to use when \n"
    "                                        parsing a directory.\n"
    "  --verbose                             Enable verbose output (info level).\n"
    "\n"
    "The parse command parses and validates the given manifests, or manifests found\n"
    "in the given directories, into an intermediate representation of the Puppet\n"
    "source code that can be used by Puppet to compile catalogs without having to\n"
    "parse the source code again. If no input manifests or directories are specified,\n"
    "the parse command will parse all manifests for the environment and any modules\n"
    "that are available for the environment.\n"
    "\n"
    "The compiler will output a file for each manifest that was parsed. By default,\n"
    "the output file is created in the same directory as the manifest that was\n"
    "parsed, but with a file extension of .xpp.\n"
    "\n"
    "If a directory is specified as an argument, the compiler will recursively search\n"
    "for all Puppet manifests under the specified directory. Use the --as-module\n"
    "option to treat directories as Puppet modules, which will search for files only\n"
    "in a 'manifests' subdirectory. The --output-subdir option can be used to create\n"
    "a subdirectory of an input directory where output files should go.\n"
    "\n"
    "If a single source manifest is specified, the --output option can be used to\n"
    "specify the full path to the output file.\n"
    ;

SCENARIO("using the parse command", "[options]")
{
    ostringstream stream;

    options::parser parser;
    parser.add<commands::help>(stream);
    parser.add<commands::parse>();

    WHEN("given an invalid option") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "parse", "--not_valid" }), option_exception);
        }
    }
    WHEN("given help is given parse as a command") {
        REQUIRE(parser.parse({ "help", "parse" }).execute() == EXIT_SUCCESS);
        THEN("it should display the help") {
            REQUIRE(stream.str() == PARSE_COMMAND_HELP);
        }
    }
    WHEN("given conflicting logging options") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "parse", "--debug", "--verbose" }), option_exception);
            REQUIRE_THROWS_AS(parser.parse({ "parse", "--debug", "-lverbose" }), option_exception);
            REQUIRE_THROWS_AS(parser.parse({ "parse", "--verbose", "--loglevel=debug" }), option_exception);
        }
    }
    WHEN("given an invalid log level") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "parse", "--loglevel=notvalid" }), option_exception);
        }
    }
    WHEN("given conflicting colorization options") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "parse", "--color", "--no-color" }), option_exception);
        }
    }
    WHEN("given a code directory that does not exist") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "parse", "--code-dir", "does_not_exist" }), option_exception);
        }
    }
    WHEN("given an environment directory that does not exist") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "parse", "--environment-dir", "does_not_exist" }), option_exception);
        }
    }
    WHEN("given an input that does not exist") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "parse", "does_not_exist" }), option_exception);
        }
    }
    WHEN("given more than one input with the output option") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "parse", "--output=foo", "bar", "baz" }), option_exception);
        }
    }
    WHEN("given an output subdirectory that is not relative") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "parse", "--output-subdir=/foo" }), option_exception);
        }
    }
    WHEN("given an output subdirectory that goes above the parent directory") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS_AS(parser.parse({ "parse", "--output-subdir=../bar" }), option_exception);
        }
    }
}
