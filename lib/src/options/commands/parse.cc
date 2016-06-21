#include <puppet/options/commands/parse.hpp>
#include <puppet/options/parser.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/environment.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/utility/filesystem/helpers.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

using namespace std;
using namespace puppet::compiler;
using namespace puppet::utility::filesystem;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace options { namespace commands {

    struct parse_stats
    {
        size_t parsed = 0;
        size_t up_to_date = 0;
        bool failed = false;
    };

    static bool needs_parse(string const& input, string const& output)
    {
        sys::error_code ec;
        auto input_last_write = fs::last_write_time(input, ec);
        if (ec) {
            return true;
        }
        auto output_last_write = fs::last_write_time(output, ec);
        if (ec) {
            return true;
        }
        return input_last_write > output_last_write;
    }

    static string get_output_path(string const& manifest, fs::path const& output_directory = {}, fs::path const& base_path = {})
    {
        fs::path output_path;
        if (output_directory.empty()) {
            output_path = manifest;
        } else {
            output_path = output_directory / (fs::path{ manifest }.lexically_relative(base_path));
        }
        output_path.replace_extension(".xpp");
        return output_path.string();
    }

    static void parse_manifest(logging::logger& logger, string const& manifest, string const& output_path, parse_stats& stats)
    {
        if (!needs_parse(manifest, output_path)) {
            LOG(debug, "skipping manifest file '%1%' because it is up-to-date.", manifest);
            ++stats.up_to_date;
            return;
        }

        // Ensure the output directory exists
        auto output_directory = fs::path{ output_path }.parent_path();
        sys::error_code ec;
        if (!fs::is_directory(output_directory, ec)) {
            if (!fs::create_directories(output_directory, ec)) {
                LOG(error, "failed to create output directory '%1%': skipping parsing manifest '%2%'.", output_directory.string(), manifest);
                stats.failed = true;
                return;
            }
        }

        ofstream stream{ output_path };
        if (!stream) {
            LOG(error, "failed to open output file '%1%' for writing.", output_path);
            stats.failed = true;
            return;
        }

        LOG(debug, "parsing manifest file '%1%'.", manifest);
        ++stats.parsed;

        try {
            auto tree = compiler::parser::parse_file(logger, manifest);

            // TODO: write in the actual XPP file format
            tree->write(compiler::ast::format::yaml, stream);
            stream << '\n';
        } catch (parse_exception const& ex) {
            compiler::lexer::line_info info;
            ifstream input{ manifest };
            if (input) {
                info = lexer::get_line_info(input, ex.begin().offset(), ex.end().offset() - ex.begin().offset());
            }
            LOG(error, ex.begin().line(), info.column, info.length, info.text, manifest, ex.what());
            // TODO: write out an XPP with the diagnostics
        }
    }

    static void parse_directory(logging::logger& logger, string const& directory, string const& output_subdirectory, bool as_module, parse_stats& stats)
    {
        auto base_path = fs::path{ directory };
        auto output_directory = fs::path{ make_absolute(output_subdirectory, base_path.string()) };

        compiler::settings temp;

        if (as_module) {
            // Treat the directory as a module using a finder
            LOG(info, "parsing directory '%1%' as module '%2%'.", directory, base_path.filename().string());
        } else {
            LOG(info, "parsing directory '%1%'.", directory);
            temp.set(settings::manifest, ".");
        }

        compiler::finder finder{ directory, as_module ? nullptr : &temp };
        finder.each_file(find_type::manifest, [&](auto const& manifest) {
            parse_manifest(logger, manifest, get_output_path(manifest, output_directory, base_path), stats);
            return true;
        });
    }

    static void parse_module(logging::logger& logger, compiler::module const& module, string const& output_subdirectory, parse_stats& stats)
    {
        LOG(info, "parsing module '%1%'.", module.name());

        auto base_path = fs::path{ module.directory() };
        auto output_directory = fs::path{ make_absolute(output_subdirectory, base_path.string()) };

        module.each_file(find_type::manifest, [&](auto const& manifest) {
            parse_manifest(logger, manifest, get_output_path(manifest, output_directory, base_path), stats);
            return true;
        });
    }

    static void parse_environment(logging::logger& logger, compiler::environment const& environment, string const& output_subdirectory, parse_stats& stats)
    {
        LOG(info, "parsing environment '%1%'.", environment.name());

        auto base_path = fs::path{ environment.directory() };
        auto output_directory = fs::path{ make_absolute(output_subdirectory, base_path.string()) };

        environment.each_file(find_type::manifest, [&](auto const& manifest) {
            parse_manifest(logger, manifest, get_output_path(manifest, output_directory, base_path), stats);
            return true;
        });

        environment.each_module([&](auto const& module) {
            parse_module(logger, module, output_subdirectory, stats);
            return true;
        });
    }

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
            "source code again. If no input manifests or directories are specified, the parse "
            "command will parse all manifests for the environment and any modules that are available for the environment."
            " <p> "
            "The compiler will output a file for each manifest that was parsed. By default, "
            "the output file is created in the same directory as the manifest that was parsed, "
            "but with a file extension of .xpp."
            " <p> "
            "If a directory is specified as an argument, the compiler will recursively search "
            "for all Puppet manifests under the specified directory. Use the --as-module option to treat directories as "
            "Puppet modules, which will search for files only in a 'manifests' subdirectory. The --output-subdir option "
            "can be used to create a subdirectory of an input directory where output files should go."
            " <p> "
            "If a single source manifest is specified, the --output option can be used to specify "
            "the full path to the output file."
            ;
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
            (AS_MODULE_OPTION, AS_MODULE_DESCRIPTION)
            (CODE_DIRECTORY_OPTION, po::value<string>(), CODE_DIRECTORY_DESCRIPTION)
            (COLOR_OPTION, COLOR_DESCRIPTION)
            (DEBUG_OPTION_FULL, DEBUG_DESCRIPTION)
            (ENVIRONMENT_OPTION_FULL, po::value<string>()->default_value("production"), ENVIRONMENT_DESCRIPTION)
            (ENVIRONMENT_PATH_OPTION, po::value<string>(), ENVIRONMENT_PATH_DESCRIPTION)
            (HELP_OPTION, HELP_DESCRIPTION)
            (LOG_LEVEL_OPTION_FULL, po::value<string>()->default_value("warning"), LOG_LEVEL_DESCRIPTION)
            (MODULE_PATH_OPTION, po::value<string>(), MODULE_PATH_DESCRIPTION)
            (NO_COLOR_OPTION, NO_COLOR_DESCRIPTION)
            (OUTPUT_OPTION_FULL, po::value<string>(), OUTPUT_DESCRIPTION)
            (OUTPUT_SUBDIR_OPTION, po::value<string>(), OUTPUT_SUBDIR_DESCRIPTION)
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

        // Get the options
        auto level = get_level(options);
        auto colorization = get_colorization(options);
        auto settings = create_settings(options);
        auto manifests = get_manifests(options);
        auto output = get_output_file(options);
        auto output_subdirectory = get_output_subdirectory(options);
        bool as_module = options.count(AS_MODULE_OPTION);

        // Validate the output option
        if (!output.empty()) {
            if (manifests.size() != 1) {
                throw option_exception("output option can only be used when parsing a single manifest file.", this);
            }
            sys::error_code ec;
            if (fs::is_directory(manifests.front(), ec)) {
                throw option_exception((boost::format("cannot use the output option when parsing directory '%1%'.") % manifests.front()).str(), this);
            }
        }

        // Move the options into the lambda capture
        return {
            *this,
            [
                level,
                colorization,
                settings = rvalue_cast(settings),
                manifests = rvalue_cast(manifests),
                output = rvalue_cast(output),
                output_subdirectory = rvalue_cast(output_subdirectory),
                as_module,
                this
            ] () {
                logging::console_logger logger;
                logger.level(level);

                // TODO: support color/no-color options
                parse_stats stats;
                try {
                    LOG(debug, "using code directory '%1%'.", settings.get(settings::code_directory));

                    if (manifests.empty()) {
                        // Create a new environment
                        auto environment = compiler::environment::create(logger, settings);
                        parse_environment(logger, *environment, output_subdirectory, stats);
                    } else {
                        if (manifests.size() == 1 && !output.empty()) {
                            parse_manifest(logger, manifests.front(), output, stats);
                        } else {
                            for (auto& manifest : manifests) {
                                sys::error_code ec;
                                if (fs::is_regular_file(manifest, ec)) {
                                    parse_manifest(logger, manifest, get_output_path(manifest), stats);
                                } else if (fs::is_directory(manifest, ec)) {
                                    parse_directory(logger, manifest, output_subdirectory, as_module, stats);
                                } else {
                                    LOG(error, "'%1%' is not a file or directory.", manifest);
                                    stats.failed = true;
                                }
                            }
                        }
                    }
                } catch (compilation_exception const& ex) {
                    throw option_exception(ex.what(), this);
                }

                auto errors = logger.errors();
                auto warnings = logger.warnings();

                LOG(info, "%1% %2% parsed and %3% %4% up-to-date.",
                    stats.parsed,
                    (stats.parsed != 1 ? "manifests" : "manifest"),
                    stats.up_to_date,
                    (stats.up_to_date != 1 ? "manifests" : "manifest")
                );
                LOG(info, "parsing %1% with %2% %3% and %4% %5%.",
                    (stats.failed ? "failed" : "succeeded"),
                    errors,
                    (errors != 1 ? "errors" : "error"),
                    warnings,
                    (warnings != 1 ? "warnings" : "warning")
                );
                return stats.failed ? EXIT_FAILURE : EXIT_SUCCESS;
            }
        };
    }

    logging::level parse::get_level(boost::program_options::variables_map const& options) const
    {
        if ((options.count(DEBUG_OPTION) + options.count(VERBOSE_OPTION) + (options[LOG_LEVEL_OPTION].defaulted() ? 0 : 1)) > 1) {
            throw option_exception((boost::format("%1%, %2%, and %3% options conflict: please specify only one.") % DEBUG_OPTION % VERBOSE_OPTION % LOG_LEVEL_OPTION).str(), this);
        }

        try
        {
            // Unlike the base command, the parse command only supports debug, info, warning, and error
            auto level = command::get_level(options);
            if (level == logging::level::debug ||
                level == logging::level::info ||
                level == logging::level::warning ||
                level == logging::level::error) {
                return level;
            }
        } catch (exception const&) {
        }
        throw option_exception(
            (boost::format("invalid log level '%1%': expected debug, info, warning, or error.") %
             options[LOG_LEVEL_OPTION].as<string>()
            ).str(),
            this
        );
    }

    vector<string> parse::get_manifests(po::variables_map const& options) const
    {
        vector<string> manifests;
        if (options.count(MANIFESTS_OPTION)) {
            for (auto& manifest : options[MANIFESTS_OPTION].as<vector<string>>()) {
                sys::error_code ec;
                if (!fs::is_regular_file(manifest, ec) && !fs::is_directory(manifest, ec)) {
                    throw option_exception((boost::format("'%1%' is not a file or directory.") % manifest).str(), this);
                }
                manifests.emplace_back(make_absolute(manifest));
            }
        }
        return manifests;
    }

    string parse::get_output_file(po::variables_map const& options) const
    {
        return options.count(OUTPUT_OPTION) ? make_absolute(options[OUTPUT_OPTION].as<string>()) : string{};
    }

    string parse::get_output_subdirectory(po::variables_map const& options) const
    {
        if (!options.count(OUTPUT_SUBDIR_OPTION)) {
            return {};
        }
        auto subdirectory = options[OUTPUT_SUBDIR_OPTION].as<string>();
        if (!normalize_relative_path(subdirectory)) {
            throw option_exception((boost::format("'%1%' cannot be used as a subdirectory because it is not contained within the parent directory.") % subdirectory).str(), this);
        }
        return subdirectory;
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
            auto environment = options[ENVIRONMENT_OPTION].as<string>();
            if (environment == "environment") {
                throw option_exception("'environment' is not a valid environment name because it conflicts with the built-in 'environment' namespace.", this);
            }
            settings.set(settings::environment, rvalue_cast(environment));
        }

        if (options.count(ENVIRONMENT_PATH_OPTION)) {
            settings.set(settings::environment_path, options[ENVIRONMENT_PATH_OPTION].as<string>());
        }

        if (options.count(MODULE_PATH_OPTION)) {
            settings.set(settings::module_path, options[MODULE_PATH_OPTION].as<string>());
        }

        return settings;
    }

    char const* const parse::AS_MODULE_OPTION             = "as-module";
    char const* const parse::AS_MODULE_DESCRIPTION        = "Parse directories as Puppet modules.";
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
    char const* const parse::OUTPUT_SUBDIR_OPTION         = "output-subdir";
    char const* const parse::OUTPUT_SUBDIR_DESCRIPTION    = "The output subdirectory to use when parsing a directory.";

}}}  // namespace puppet::options::commands
