#include <puppet/options/commands/compile.hpp>
#include <puppet/options/parser.hpp>
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
    static char const* const MODULE_PATH_OPTION = "module-path";
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

        // If no node name was specified, use the FQDN fact and fallback to "<hostname>[.<domain>]".
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
        return options.count(MANIFESTS_OPTION) ? options[MANIFESTS_OPTION].as<vector<string>>() : vector<string>{};
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

    static settings create_settings(options::command const& command, po::variables_map const& options)
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
                throw option_exception((boost::format("code directory '%1%' does not exist or is not a directory.") % directory).str(), &command);
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
            "Compiles one or more Puppet manifests, or directories containing manifests, into a Puppet catalog."
            " <p> "
            "When invoked with no options, the compiler will compile a catalog for the 'production' environment."
            " <p> "
            "Manifests will be evaluated in the order they are presented on the command line.";
    }

    char const* compile::arguments() const
    {
        return "[[manifest | directory] ...]";
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
                "The list of paths to use for finding environments."
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
                MODULE_PATH_OPTION,
                po::value<string>(),
                "The list of paths to use for finding modules."
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
        auto settings = create_settings(*this, options);
        auto node_name = get_node(*this, options, *facts);
        auto manifests = get_manifests(*this, options);

        // Move the options into the lambda capture
        return {
            *this,
            [
                level,
                settings = rvalue_cast(settings),
                manifests = rvalue_cast(manifests),
                node_name = rvalue_cast(node_name),
                facts = rvalue_cast(facts),
                output_file = rvalue_cast(output_file),
                graph_file = rvalue_cast(graph_file),
                this
            ] () {
                bool failed = true;
                logging::console_logger logger;

                try {
                    logger.level(level);

                    // TODO: support color/no-color options

                    LOG(debug, "using code directory '%1%'.", settings.get(settings::code_directory));

                    // Create a new environment with the builtin functions and operators
                    auto environment = compiler::environment::create(logger, settings, manifests);
                    environment->dispatcher().add_builtins();

                    if (environment->manifests().empty()) {
                        throw option_exception(
                            (boost::format("no manifests were found for environment '%1%': expected at least one manifest as an argument.") %
                             name()
                            ).str(),
                            this
                        );
                    }

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

                        // Command succeeded
                        failed = false;

                    } catch (compilation_exception const& ex) {
                        LOG(error, ex.line(), ex.column(), ex.length(), ex.text(), ex.path(), "node '%1%': %2%", node.name(), ex.what());
                    } catch (resource_cycle_exception const& ex) {
                        LOG(error, ex.what());
                    }
                } catch (compilation_exception const& ex) {
                    LOG(error, ex.line(), ex.column(), ex.length(), ex.text(), ex.path(), ex.what());
                } catch (yaml_parse_exception const& ex) {
                    LOG(error, ex.line(), 1, ex.column(), ex.text(), ex.path(), ex.what());
                } catch (exception const& ex) {
                    LOG(critical, "unhandled exception: %1%", ex.what());
                }

                auto errors = logger.errors();
                auto warnings = logger.warnings();

                LOG(notice, "compilation %1% with %2% %3% and %4% %5%.",
                    (failed ? "failed" : "succeeded"),
                    errors,
                    (errors != 1 ? "errors" : "error"),
                    warnings,
                    (warnings != 1 ? "warnings" : "warning")
                );
                return failed ? EXIT_FAILURE : EXIT_SUCCESS;
            }
        };
    }

}}}  // namespace puppet::options::commands
