#include <puppet/options/commands/compile.hpp>
#include <puppet/options/parser.hpp>
#include <puppet/options/defaults.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/facts/facter.hpp>
#include <puppet/facts/yaml.hpp>
#include <puppet/logging/logger.hpp>
#include <puppet/utility/filesystem/helpers.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

using namespace std;
using namespace puppet::facts;
using namespace puppet::runtime;
using namespace puppet::compiler;
using namespace puppet::utility::filesystem;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace options { namespace commands {

    static char const* const CODE_DIRECTORY_OPTION = "code-dir";
    static char const* const COLOR_OPTION = "color";
    static char const* const DEBUG_OPTION = "debug";
    static char const* const DEBUG_OPTION_FULL = "debug,d";
    static char const* const ENVIRONMENT_OPTION = "environment";
    static char const* const ENVIRONMENT_OPTION_FULL = "environment,e";
    static char const* const ENVIRONMENT_PATH_OPTION = "environment-path";
    static char const* const FACTS_OPTION = "facts";
    static char const* const FACTS_OPTION_FULL = "facts,f";
    static char const* const GRAPH_OPTION = "graph";
    static char const* const GRAPH_OPTION_FULL = "graph,g";
    static char const* const HELP_OPTION = "help";
    static char const* const LOG_LEVEL_OPTION = "log-level";
    static char const* const LOG_LEVEL_OPTION_FULL = "log-level,l";
    static char const* const MANIFESTS_OPTION = "manifests";
    static char const* const MODULE_DIRECTORY_OPTION = "module-dir";
    static char const* const NODE_OPTION = "node";
    static char const* const NODE_OPTION_FULL = "node,n";
    static char const* const NO_COLOR_OPTION = "no-color";
    static char const* const OUTPUT_OPTION = "output";
    static char const* const OUTPUT_OPTION_FULL = "output,o";
    static char const* const VERBOSE_OPTION = "verbose";

    static logging::level get_level(options::command const& command, po::variables_map const& options)
    {
        // Check for conflicting options
        if ((options.count(DEBUG_OPTION) + options.count(VERBOSE_OPTION) + (options[LOG_LEVEL_OPTION].defaulted() ? 0 : 1)) > 1) {
            throw option_exception((boost::format("%1%, %2%, and %3% options conflict: please specify only one.") % DEBUG_OPTION % VERBOSE_OPTION % LOG_LEVEL_OPTION).str(), &command);
        }

        // Override the log level for debug/verbose
        if (options.count(DEBUG_OPTION)) {
            return logging::level::debug;
        }
        if (options.count(VERBOSE_OPTION)) {
            return logging::level::info;
        }
        return options[LOG_LEVEL_OPTION].as<logging::level>();
    }

    static boost::optional<bool> get_colorization(options::command const& command, po::variables_map const& options)
    {
        if (options.count(COLOR_OPTION) && options.count(NO_COLOR_OPTION)) {
            throw option_exception((boost::format("%1% and %2% options conflict: please specify only one.") % COLOR_OPTION % NO_COLOR_OPTION).str(), &command);
        }
        if (!options.count(COLOR_OPTION) && !options.count(NO_COLOR_OPTION)) {
            return boost::none;
        }
        return options.count(COLOR_OPTION) > 0;
    }

    static string get_code_directory(options::command const& command, po::variables_map const& options)
    {
        string directory;
        if (options.count(CODE_DIRECTORY_OPTION)) {
            directory = options[CODE_DIRECTORY_OPTION].as<string>();
        }
        if (directory.empty()) {
            directory = defaults::code_directory();
        } else {
            directory = make_absolute(directory);

            // Ensure the directory exists
            sys::error_code ec;
            if (!fs::is_directory(directory, ec) || ec) {
                throw option_exception((boost::format("code directory '%1%' does not exist or is not a directory.") % directory).str(), &command);
            }
        }
        return directory;
    }

    static string get_environment(po::variables_map const& options)
    {
        return options[ENVIRONMENT_OPTION].as<string>();
    }

    static shared_ptr<facts::provider> get_facts(po::variables_map const& options)
    {
        if (options.count(FACTS_OPTION)) {
            return make_shared<facts::yaml>(options[FACTS_OPTION].as<string>());
        }

        // Default to facter
        return make_shared<facts::facter>();
    }

    static string get_node(options::command const& command, po::variables_map const& options, facts::provider& facts)
    {
        // Check to see if it was explicitly set
        string name;
        if (options.count(NODE_OPTION)) {
            name = options[NODE_OPTION].as<string>();
        }
        if (!name.empty()) {
            return name;
        }

        // If no node name was specified, use the FQDN fact
        // NOTE: the following uses of boost::get are safe because facts never contain runtime variables
        // Next try "networking" fact
        auto networking = facts.lookup("networking");
        if (networking) {
            if (auto hash = networking->as<runtime::values::hash>()) {
                auto fqdn = hash->get("fqdn");
                if (fqdn) {
                    if (auto str = fqdn->as<string>()) {
                        name = *str;
                    }
                }
                // Fallback to the hostname and domain if present
                if (name.empty()) {
                    auto hostname = hash->get("hostname");
                    if (hostname) {
                        if (auto str = hostname->as<string>()) {
                            name = *str;
                        }
                        if (!name.empty()) {
                            auto domain = hash->get("domain");
                            if (domain) {
                                if (auto str = domain->as<string>()) {
                                    name += "." + *str;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Next try the legacy fqdn fact
        if (name.empty()) {
            auto fqdn = facts.lookup("fqdn");
            if (fqdn) {
                if (auto str = fqdn->as<string>()) {
                    name = *str;
                }
            }
        }

        // Next try legacy hostname + domain facts
        if (name.empty()) {
            auto hostname = facts.lookup("hostname");
            if (hostname) {
                if (auto str = hostname->as<string>()) {
                    name = *str;
                }
                if (!name.empty()) {
                    auto domain = facts.lookup("domain");
                    if (domain) {
                        if (auto str = domain->as<string>()) {
                            name += "." + *str;
                        }
                    }
                }
            }
        }

        // If still empty, user must explicitly specify
        if (name.empty()) {
            throw option_exception((boost::format("node name cannot be determined from facts: please specify the --%1% option to set the node name.") % NODE_OPTION).str(), &command);
        }
        return name;
    }

    static vector<string> get_manifests(options::command const& command, po::variables_map const& options)
    {
        vector<string> manifests;
        if (options.count(MANIFESTS_OPTION)) {
            manifests = options[MANIFESTS_OPTION].as<vector<string>>();
            for (auto& manifest : manifests) {
                auto absolute_path = make_absolute(manifest);

                // Ensure the manifest is a file (TODO: remove this check when we support directories from the command line)
                sys::error_code ec;
                if (!fs::is_regular_file(absolute_path, ec) || ec) {
                    throw option_exception((boost::format("path '%1%' is not a manifest file.") % manifest).str(), &command);
                }
                manifest = rvalue_cast(absolute_path);
            }
        }
        return manifests;
    }

    static string get_output_file(po::variables_map const& options)
    {
        return make_absolute(options[OUTPUT_OPTION].as<string>());
    }

    static string get_graph_file(po::variables_map const& options)
    {
        if (options.count(GRAPH_OPTION)) {
            return make_absolute(options[GRAPH_OPTION].as<string>());
        }
        return {};
    }

    static string get_environment_directory(options::command const& command, po::variables_map const& options, string const& code_directory, string const& environment)
    {
        bool specified = false;
        string search_path;
        if (options.count(ENVIRONMENT_PATH_OPTION)) {
            search_path = options[ENVIRONMENT_PATH_OPTION].as<string>();
            specified = true;
        } else {
            search_path = defaults::environment_path();
        }

        string default_directory;
        boost::split_iterator<string::iterator> end;
        for (auto it = boost::make_split_iterator(search_path, boost::first_finder(path_separator(), boost::is_equal())); it != end; ++it) {
            if (!*it) {
                continue;
            }

            string directory{ it->begin(), it->end() };
            boost::replace_all(directory, "$codedir", code_directory);
            auto path = fs::path{ make_absolute(directory) } / environment;

            sys::error_code ec;
            if (fs::is_directory(path, ec)) {
                return path.string();
            }

            if (default_directory.empty()) {
                default_directory = path.string();
            }
        }

        // If directories were specified, it's an error if the environment's directory cannot be found
        if (specified || default_directory.empty()) {
            throw option_exception(
                (boost::format("could not locate an environment directory for environment '%1%' using search path '%2%'.") %
                 environment %
                 search_path
                ).str(),
                &command);
        }

        // Return the first directory that would have been in the path
        return default_directory;
    }

    static vector<string> get_module_directories(po::variables_map const& options, string const& code_directory)
    {
        vector<string> directories;
        if (options.count(MODULE_DIRECTORY_OPTION)) {
            directories = options[MODULE_DIRECTORY_OPTION].as<vector<string>>();
        } else {
            directories = defaults::module_directories();
        }

        for (auto& directory : directories) {
            // Replace all references to $codedir with the code directory
            boost::replace_all(directory, "$codedir", code_directory);
            directory = make_absolute(directory);

            sys::error_code ec;
            if (!fs::is_directory(directory, ec) || ec) {
                directory.clear();
            }
        }

        // Remove all empty directories
        directories.erase(
            remove_if(directories.begin(), directories.end(), [](string const& directory) { return directory.empty(); }),
            directories.end());
        return directories;
    }

    char const* compile::name() const
    {
        return "compile";
    }

    char const* compile::description() const
    {
        return "Compile Puppet manifests into a Puppet catalog.";
    }

    char const* compile::summary() const
    {
        return
            "Compiles a Puppet manifest into a Puppet catalog. When invoked with no options, "
            "the compiler will compile the manifest for the 'production' environment."
            " <p> "
            "Manifests will be evaluated in the order they are presented on the command line.";
    }

    char const* compile::arguments() const
    {
        return "[[manifest] [manifest] ...]";
    }

    po::options_description compile::create_options() const
    {
        // Keep this list sorted alphabetically on full option name
        po::options_description options("");
        options.add_options()
            (
                CODE_DIRECTORY_OPTION,
                po::value<string>(),
                "The Puppet code directory to use. Defaults to the current platform's code directory."
            )
            (
                COLOR_OPTION,
                "Forces color output on platforms that support colorized output."
            )
            (
                DEBUG_OPTION_FULL,
                "Enable debug output."
            )
            (
                ENVIRONMENT_OPTION_FULL,
                po::value<string>()->default_value("production"),
                "The environment to compile for."
            )
            (
                ENVIRONMENT_PATH_OPTION,
                po::value<string>(),
                "The search path to use for finding environments."
            )
            (
                FACTS_OPTION_FULL,
                po::value<string>(),
                "The path to the YAML facts file to use. Defaults to the current system's facts."
            )
            (
                GRAPH_OPTION_FULL,
                po::value<string>(),
                "The path to write a DOT language file for viewing the catalog dependency graph."
            )
            (
                HELP_OPTION,
                "Display command help."
            )
            (
                LOG_LEVEL_OPTION_FULL,
                po::value<logging::level>()->default_value(logging::level::notice, "notice"),
                "Set logging level.\nSupported levels: debug, info, notice, warning, error, alert, emergency, critical."
            )
            (
                MODULE_DIRECTORY_OPTION,
                po::value<vector<string>>(),
                "Specifies a directory to search for global modules."
            )
            (
                NODE_OPTION_FULL,
                po::value<string>(),
                "The node name to use. Defaults to the 'fqdn' fact."
            )
            (
                NO_COLOR_OPTION,
                "Disables color output."
            )
            (
                OUTPUT_OPTION_FULL,
                po::value<string>()->default_value("catalog.json"),
                "The output path for the compiled catalog."
            )
            (
                VERBOSE_OPTION,
                "Enable verbose (info) output."
            )
            ;
        return options;
    }

    po::options_description compile::create_hidden_options() const
    {
        po::options_description options;
        options.add_options()
            (MANIFESTS_OPTION, po::value<vector<string>>())
            ;
        return options;
    }

    po::positional_options_description compile::create_positional_options() const
    {
        po::positional_options_description options;
        options.add(MANIFESTS_OPTION, -1);
        return options;
    }

    executor compile::create_executor(po::variables_map const& options) const
    {
        if (options.count(HELP_OPTION)) {
            return parser().parse({ HELP_OPTION, name() });
        }

        // Get the options
        auto level = get_level(*this, options);
        auto colorization = get_colorization(*this, options);
        auto output_file = get_output_file(options);
        auto graph_file = get_graph_file(options);
        auto facts = get_facts(options);
        auto node_name = get_node(*this, options, *facts);
        auto code_directory = get_code_directory(*this, options);
        auto module_directories = get_module_directories(options, code_directory);
        auto environment_name = get_environment(options);
        auto environment_directory = get_environment_directory(*this, options, code_directory, environment_name);
        auto manifests = get_manifests(*this, options);

        // Move the options into the lambda capture
        return {
            *this,
            [
                level,
                code_directory = rvalue_cast(code_directory),
                environment_name = rvalue_cast(environment_name),
                environment_directory = rvalue_cast(environment_directory),
                module_directories = rvalue_cast(module_directories),
                manifests = rvalue_cast(manifests),
                node_name = rvalue_cast(node_name),
                facts = rvalue_cast(facts),
                output_file = rvalue_cast(output_file),
                graph_file = rvalue_cast(graph_file),
                this
            ] () {
                logging::console_logger logger;

                try {
                    logger.level(level);

                    // TODO: support color/no-color options

                    // Create an environment
                    auto environment = make_shared<compiler::environment>(environment_name, environment_directory, manifests);

                    // Log some useful information for debugging purposes before doing anything
                    if (logger.would_log(logging::level::debug)) {
                        LOG(debug, "using directory '%1%' as the code directory.", code_directory);
                        LOG(debug, "using directory '%1%' as the environment directory.", environment->directory());
                        for (auto const& directory : module_directories) {
                            LOG(debug, "using directory '%1%' to search for global modules.", directory);
                        }
                    }

                    // Load the modules into the environment
                    environment->load_modules(logger, module_directories);

                    // Construct a node
                    compiler::node node{logger, node_name, environment, facts};

                    // Open the output file for writing
                    ofstream output{ output_file };
                    if (!output) {
                        throw option_exception((boost::format("cannot open '%1%' for writing.") % output_file).str(), this);
                    }

                    try {
                        LOG(notice, "compiling for node '%1%' with environment '%2%'.", node.name(), node.environment().name());

                        // Compile the node
                        auto catalog = node.compile();

                        // Write the graph file if given one
                        if (!graph_file.empty()) {
                            ofstream file{ graph_file };
                            if (!file) {
                                LOG(error, "cannot open '%1%' for writing.", graph_file);
                            } else {
                                LOG(notice, "writing dependency graph to '%1%'.", graph_file);
                                catalog.write_graph(file);
                            }
                        }

                        // Detect dependency cycles
                        catalog.detect_cycles();

                        // Write the catalog
                        LOG(notice, "writing catalog to '%1%'.", output_file);
                        catalog.write(output);

                    } catch (compilation_exception const& ex) {
                        LOG(error, ex.line(), ex.column(), ex.length(), ex.text(), ex.path(), "node '%1%': %2%", node.name(), ex.what());
                    } catch (resource_cycle_exception const& ex) {
                        LOG(error, ex.what());
                    }
                } catch (yaml_parse_exception const& ex) {
                    LOG(error, ex.line(), 1, ex.column(), ex.text(), ex.path(), ex.what());
                } catch (exception const& ex) {
                    LOG(critical, "unhandled exception: %1%", ex.what());
                }

                auto errors = logger.errors();
                auto warnings = logger.warnings();

                LOG(notice, "compilation %1% with %2% %3% and %4% %5%.",
                    (logger.failed() ? "failed" : "succeeded"),
                    errors,
                    (errors != 1 ? "errors" : "error"),
                    warnings,
                    (warnings != 1 ? "warnings" : "warning")
                );
                return logger.failed() ? EXIT_FAILURE : EXIT_SUCCESS;
            }
        };
    }

}}}  // namespace puppet::options::commands
