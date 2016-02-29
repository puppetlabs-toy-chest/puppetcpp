#include <puppet/options/commands/parse.hpp>
#include <puppet/options/parser.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/utility/filesystem/helpers.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::compiler;
using namespace puppet::utility::filesystem;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace options { namespace commands {

    char const* parse::name() const
    {
        return "parse";
    }

    char const* parse::description() const
    {
        return "Parse Puppet manifests into intermediate representations (XPP).";
    }

    char const* parse::summary() const
    {
        return
            "The parse command parses and validates the given manifests, or manifests found in "
            "the given directories, into an intermediate representation of the Puppet source "
            "code that can be used by Puppet to compile catalogs without having to parse the "
            "source code again."
            " <p> "
            "The compiler will output a file for each manifest that was parsed. By default, "
            "the output file is created in the same directory as the manifest that was parsed, "
            "but with a file extension of .xpp."
            " <p> "
            "If a directory is specified as an argument, the compiler will recursively search "
            "for Puppet manifests under the specified directory.  The --output-subdir option can be "
            "used to create a subdirectory of the input directory where output files should go."
            " <p> "
            "If a single source manifest is specified, the --output option can be used to specify "
            "the full path to the output file.";
    }

    char const* parse::arguments() const
    {
        return "[[manifest | directory] ...]";
    }

    po::options_description parse::create_options() const
    {
        // Keep this list sorted alphabetically on full option name
        po::options_description options("");
        options.add_options()
            (CODE_DIRECTORY_OPTION, po::value<string>(), CODE_DIRECTORY_DESCRIPTION)
            (COLOR_OPTION, COLOR_DESCRIPTION)
            (DEBUG_OPTION_FULL, DEBUG_DESCRIPTION)
            (ENVIRONMENT_OPTION_FULL, po::value<string>()->default_value("production"), ENVIRONMENT_DESCRIPTION)
            (ENVIRONMENT_PATH_OPTION, po::value<string>(), ENVIRONMENT_PATH_DESCRIPTION)
            (HELP_OPTION, HELP_DESCRIPTION)
            (LOG_LEVEL_OPTION_FULL, po::value<logging::level>()->default_value(logging::level::warning, "warning"), LOG_LEVEL_DESCRIPTION)
            (MODULE_PATH_OPTION, po::value<string>(), MODULE_PATH_DESCRIPTION)
            (NO_COLOR_OPTION, NO_COLOR_DESCRIPTION)
            (OUTPUT_OPTION_FULL, po::value<string>(), OUTPUT_DESCRIPTION)
            (VERBOSE_OPTION, VERBOSE_DESCRIPTION)
            ;
        return options;
    }

    po::options_description parse::create_hidden_options() const
    {
        po::options_description options;
        options.add_options()
            (MANIFESTS_OPTION, po::value<vector<string>>())
            ;
        return options;
    }

    po::positional_options_description parse::create_positional_options() const
    {
        po::positional_options_description options;
        options.add(MANIFESTS_OPTION, -1);
        return options;
    }

    executor parse::create_executor(po::variables_map const& options) const
    {
        if (options.count(HELP_OPTION)) {
            return parser().parse({ HELP_OPTION, name() });
        }

        // Move the options into the lambda capture
        return {
            *this,
            [
                this
            ] () {
                throw runtime_error("not yet implemented.");
                return EXIT_FAILURE;
            }
        };
    }

    vector<string> parse::get_manifests(po::variables_map const& options) const
    {
        return options.count(MANIFESTS_OPTION) ? options[MANIFESTS_OPTION].as<vector<string>>() : vector<string>{};
    }

    string parse::get_output_file(po::variables_map const& options) const
    {
        return options.count(OUTPUT_OPTION) ? make_absolute(options[OUTPUT_OPTION].as<string>()) : string{};
    }

    settings parse::create_settings(po::variables_map const& options) const
    {
        compiler::settings settings;

        if (options.count(MODULE_PATH_OPTION)) {
            settings.set(settings::base_module_path, options[MODULE_PATH_OPTION].as<string>());
        }

        if (options.count(CODE_DIRECTORY_OPTION)) {
            auto directory = make_absolute(options[CODE_DIRECTORY_OPTION].as<string>());

            // Ensure the directory exists
            sys::error_code ec;
            if (!fs::is_directory(directory, ec) || ec) {
                throw option_exception((boost::format("code directory '%1%' does not exist or is not a directory.") % directory).str(), this);
            }
            settings.set(settings::code_directory, rvalue_cast(directory));
        }

        if (options.count(ENVIRONMENT_OPTION)) {
            settings.set(settings::environment, options[ENVIRONMENT_OPTION].as<string>());
        }

        if (options.count(ENVIRONMENT_PATH_OPTION)) {
            settings.set(settings::environment_path, options[ENVIRONMENT_PATH_OPTION].as<string>());
        }

        if (options.count(MODULE_PATH_OPTION)) {
            settings.set(settings::module_path, options[MODULE_PATH_OPTION].as<string>());
        }

        return settings;
    }

    char const* const parse::CODE_DIRECTORY_OPTION        = "code-dir";
    char const* const parse::CODE_DIRECTORY_DESCRIPTION   = "The Puppet code directory to use. Defaults to the current platform's code directory.";
    char const* const parse::ENVIRONMENT_OPTION           = "environment";
    char const* const parse::ENVIRONMENT_OPTION_FULL      = "environment,e";
    char const* const parse::ENVIRONMENT_DESCRIPTION      = "The environment to use.";
    char const* const parse::ENVIRONMENT_PATH_OPTION      = "environment-path";
    char const* const parse::ENVIRONMENT_PATH_DESCRIPTION = "The list of paths to use for finding environments.";
    char const* const parse::LOG_LEVEL_DESCRIPTION        = "Set logging level.\nSupported levels: debug, info, warning, error.";
    char const* const parse::MANIFESTS_OPTION             = "manifests";
    char const* const parse::MODULE_PATH_OPTION           = "module-path";
    char const* const parse::MODULE_PATH_DESCRIPTION      = "The list of paths to use for finding modules.";
    char const* const parse::OUTPUT_OPTION                = "output";
    char const* const parse::OUTPUT_OPTION_FULL           = "output,o";
    char const* const parse::OUTPUT_DESCRIPTION           = "The output path when parsing a single input manifest.";

}}}  // namespace puppet::options::commands
