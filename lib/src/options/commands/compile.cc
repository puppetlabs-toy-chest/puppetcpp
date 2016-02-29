#include <puppet/options/commands/compile.hpp>
#include <puppet/options/parser.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/facts/facter.hpp>
#include <puppet/facts/yaml.hpp>
#include <puppet/utility/filesystem/helpers.hpp>
#include <boost/filesystem.hpp>
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
            (CODE_DIRECTORY_OPTION, po::value<string>(), CODE_DIRECTORY_DESCRIPTION)
            (COLOR_OPTION, COLOR_DESCRIPTION)
            (DEBUG_OPTION_FULL, DEBUG_DESCRIPTION)
            (ENVIRONMENT_OPTION_FULL, po::value<string>()->default_value("production"), ENVIRONMENT_DESCRIPTION)
            (ENVIRONMENT_PATH_OPTION, po::value<string>(), ENVIRONMENT_PATH_DESCRIPTION)
            (FACTS_OPTION_FULL, po::value<string>(), FACTS_DESCRIPTION)
            (GRAPH_FILE_OPTION_FULL, po::value<string>(), GRAPH_FILE_DESCRIPTION)
            (HELP_OPTION, HELP_DESCRIPTION)
            (LOG_LEVEL_OPTION_FULL, po::value<logging::level>()->default_value(logging::level::notice, "notice"), command::LOG_LEVEL_DESCRIPTION)
            (MODULE_PATH_OPTION, po::value<string>(), MODULE_PATH_DESCRIPTION)
            (NODE_OPTION_FULL, po::value<string>(), NODE_DESCRIPTION)
            (NO_COLOR_OPTION, NO_COLOR_DESCRIPTION)
            (OUTPUT_OPTION_FULL, po::value<string>()->default_value("catalog.json"), OUTPUT_DESCRIPTION)
            (VERBOSE_OPTION, VERBOSE_DESCRIPTION)
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
        auto level = get_level(options);
        auto colorization = get_colorization(options);
        auto output_file = get_output_file(options);
        auto graph_file = get_graph_file(options);
        auto facts = get_facts(options);
        auto settings = create_settings(options);
        auto node_name = get_node(options, *facts);
        auto manifests = get_manifests(options);

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

    shared_ptr<facts::provider> compile::get_facts(po::variables_map const& options) const
    {
        if (options.count(FACTS_OPTION)) {
            return make_shared<facts::yaml>(options[FACTS_OPTION].as<string>());
        }

        // Default to facter
        return make_shared<facts::facter>();
    }

    string compile::get_node(po::variables_map const& options, facts::provider& facts) const
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
            throw option_exception(
                (boost::format("node name cannot be determined from facts: please specify the --%1% option to set the node name.") %
                 NODE_OPTION
                ).str(), this);
        }
        return name;
    }

    string compile::get_graph_file(po::variables_map const& options) const
    {
        if (options.count(GRAPH_FILE_OPTION)) {
            return make_absolute(options[GRAPH_FILE_OPTION].as<string>());
        }
        return {};
    }

    char const* const compile::FACTS_OPTION           = "facts";
    char const* const compile::FACTS_OPTION_FULL      = "facts,f";
    char const* const compile::FACTS_DESCRIPTION      = "The path to the YAML facts file to use. Defaults to the current system's facts.";
    char const* const compile::GRAPH_FILE_OPTION      = "graph-file";
    char const* const compile::GRAPH_FILE_OPTION_FULL = "graph-file,g";
    char const* const compile::GRAPH_FILE_DESCRIPTION = "The path to write a DOT language file for viewing the catalog dependency graph.";
    char const* const compile::NODE_OPTION            = "node";
    char const* const compile::NODE_OPTION_FULL       = "node,n";
    char const* const compile::NODE_DESCRIPTION       = "The node name to use. Defaults to the 'fqdn' fact.";
    char const* const compile::OUTPUT_DESCRIPTION     = "The output path for the compiled catalog.";

}}}  // namespace puppet::options::commands
