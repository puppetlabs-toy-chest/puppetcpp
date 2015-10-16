#include <puppet/compiler/settings.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/facts/facter.hpp>
#include <puppet/facts/yaml.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
// boost includes are not always warning-clean. Disable warnings that
// cause problems before including the headers, then re-enable the warnings.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop
#include <iostream>

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace compiler {

    static po::options_description create_options()
    {
        // Build a list of options visible on the command line
        // Keep this list sorted alphabetically based on each option's long-form name
        po::options_description options("");
        options.add_options()
            (
                "code-dir",
                po::value<string>(),
                "The Puppet code directory to use. Defaults to the current platform's code directory."
            )
            (
                "color",
                "Forces color output on platforms that support colorized output."
            )
            (
                "debug,d",
                "Enable debug output."
            )
            (
                "environment,e",
                po::value<string>()->default_value("production"),
                "The environment to compile for."
            )
            (
                "environment-dir",
                po::value<vector<string>>(),
                "Specifies a directory to search for environments."
            )
            (
                "facts,f",
                po::value<string>(),
                "The path to the YAML facts file to use. Defaults to the current system's facts."
            )
            (
                "graph,g",
                po::value<string>(),
                "The path to write a DOT language file for viewing the catalog depencency graph."
            )
            (
                "help",
                "Print this help message."
            )
            (
                "log-level,l",
                po::value<logging::level>()->default_value(logging::level::notice, "notice"),
                "Set logging level.\nSupported levels: debug, info, notice, warning, error, alert, emergency, critical."
            )
            (
                "module-dir",
                po::value<vector<string>>(),
                "Specifies a directory to search for global modules."
            )
            (
                "node,n",
                po::value<string>(),
                "The node name to use. Defaults to the 'fqdn' fact."
            )
            (
                "no-color",
                "Disables color output."
            )
            (
                "output,o",
                po::value<string>()->default_value("catalog.json"),
                "The output path for the compiled catalog."
            )
            (
                "verbose",
                "Enable verbose (info) output."
            )
            (
                "version,v",
                "Print the version and exit."
            )
            ;
        return options;
    }

    static logging::level get_level(po::variables_map const& vm)
    {
        // Check for conflicting options
        if (vm.count("color") && vm.count("no-color")) {
            throw settings_exception("color and no-color options conflict: please specify only one.");
        }
        if ((vm.count("debug") + vm.count("verbose") + (vm["log-level"].defaulted() ? 0 : 1)) > 1) {
            throw settings_exception("debug, verbose, and log-level options conflict: please specify only one.");
        }

        // Override the log level for debug/verbose
        if (vm.count("debug")) {
            return logging::level::debug;
        }
        if (vm.count("verbose")) {
            return logging::level::info;
        }
        return vm["log-level"].as<logging::level>();
    }

    static string get_code_directory(po::variables_map const& vm)
    {
        string directory;
        if (vm.count("code-dir")) {
            directory = vm["code-dir"].as<string>();
        }
        if (directory.empty()) {
            directory = settings::default_code_directory();
        } else {
            // Not default, so ensure the path exists by getting the code path
            sys::error_code ec;
            auto path = fs::canonical(directory, ec);
            if (ec) {
                throw settings_exception((boost::format("invalid code directory '%1%': %2%.") % directory % ec.message()).str());
            }
            directory = path.string();
        }
        return directory;
    }

    static string get_environment(po::variables_map const& vm)
    {
        return vm["environment"].as<string>();
    }

    static shared_ptr<facts::provider> get_facts(po::variables_map const& vm)
    {
        if (vm.count("facts")) {
            return make_shared<facts::yaml>(vm["facts"].as<string>());
        }

        // Default to facter
        return make_shared<facts::facter>();
    }

    static string get_node(po::variables_map const& vm, facts::provider& facts)
    {
        // Check to see if it was explicitly set
        string name;
        if (vm.count("node")) {
            name = vm["node"].as<string>();
        }
        if (!name.empty()) {
            return name;
        }

        // If no node name was specified, use the FQDN fact
        // NOTE: the following uses of boost::get are safe because facts never contain runtime variables
        // Next try "networking" fact
        auto networking = facts.lookup("networking");
        if (networking) {
            if (auto ptr = boost::get<runtime::values::hash const>(networking.get())) {
                auto it = ptr->find(string("fqdn"));
                if (it != ptr->end()) {
                    if (auto str = boost::get<string const>(&it->second)) {
                        name = *str;
                    }
                }
                // Fallback to the hostname and domain if present
                if (name.empty()) {
                    auto hostname = ptr->find(string("hostname"));
                    if (hostname != ptr->end()) {
                        if (auto str = boost::get<string const>(&hostname->second)) {
                            name = *str;
                        }
                        if (!name.empty()) {
                            auto domain = ptr->find(string("domain"));
                            if (domain != ptr->end()) {
                                if (auto str = boost::get<string const>(&domain->second)) {
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
                if (auto ptr = boost::get<string const>(fqdn.get())) {
                    name = *ptr;
                }
            }
        }

        // Next try legacy hostname + domain facts
        if (name.empty()) {
            auto hostname = facts.lookup("hostname");
            if (hostname) {
                if (auto ptr = boost::get<string const>(hostname.get())) {
                    name = *ptr;
                }
                if (!name.empty()) {
                    auto domain = facts.lookup("domain");
                    if (domain) {
                        if (auto ptr = boost::get<string const>(domain.get())) {
                            name += "." + *ptr;
                        }
                    }
                }
            }
        }

        // If still empty, user must explicitly specify
        if (name.empty()) {
            throw settings_exception("node name cannot be determined from facts: please specify the --node option to set the node name.");
        }
        return name;
    }

    static vector<string> get_manifests(po::variables_map const& vm)
    {
        vector<string> manifests;
        if (vm.count("manifests")) {
            manifests = vm["manifests"].as<vector<string>>();
            for (auto& manifest : manifests) {
                manifest = fs::absolute(manifest).string();
            }
        }
        return manifests;
    }

    static string get_output_file(po::variables_map const& vm)
    {
        if (vm.count("output")) {
            return vm["output"].as<string>();
        }
        return {};
    }

    static string get_graph_file(po::variables_map const& vm)
    {
        if (vm.count("graph")) {
            return vm["graph"].as<string>();
        }
        return {};
    }

    static string get_environment_directory(po::variables_map const& vm, string const& code_directory, string const& environment)
    {
        bool specified = false;
        vector<string> directories;
        if (vm.count("environment-dir")) {
            directories = vm["environment-dir"].as<vector<string>>();
            specified = true;
        } else {
            directories = settings::default_environment_directories();
        }

        for (auto& directory : directories) {
            // Replace all references to $codedir with the code directory
            boost::replace_all(directory, "$codedir", code_directory);

            sys::error_code ec;
            auto path = fs::canonical(directory, ec);
            if (ec) {
                // TODO: it'd be nice to log that the directory does not exist
                continue;
            }

            path /= environment;
            if (fs::exists(path, ec)) {
                return path.string();
            }

            // TODO: it'd be nice to log that the directory does not exist
        }

        // If directories were specified, it's an error if the environment's directory cannot be found
        if (specified) {
            throw settings_exception((boost::format("could not locate an environment directory for environment '%1%'.") % environment).str());
        }

        // Use the default
        return (fs::path(directories.back()) / environment).string();
    }

    static vector<string> get_module_directories(po::variables_map const& vm, string const& code_directory)
    {
        vector<string> directories;
        if (vm.count("module-dir")) {
            directories = vm["module-dir"].as<vector<string>>();
        } else {
            directories = settings::default_module_directories();
        }

        for (auto& directory : directories) {
            // Replace all references to $codedir with the code directory
            boost::replace_all(directory, "$codedir", code_directory);

            sys::error_code ec;
            auto path = fs::canonical(directory, ec);
            if (ec) {
                // TODO: it'd be nice to log that the directory does not exist
                directory.clear();
                continue;
            }
            directory = path.string();
        }

        // Remove all empty directories
        directories.erase(
            remove_if(directories.begin(), directories.end(), [](string const& directory) { return directory.empty(); }),
            directories.end());
        return directories;
    }

    settings::settings() :
        _log_level(logging::level::notice),
        _show_help(false),
        _show_version(false)
    {
        char const* argv[] = { "puppetcpp" };
        parse(sizeof(argv) / sizeof(char const*), argv);
    }

    settings::settings(int argc, char const* argv[]) :
        _log_level(logging::level::notice),
        _show_help(false),
        _show_version(false)
    {
        parse(argc, argv);
    }

    string const& settings::code_directory() const
    {
        return _code_directory;
    }

    string const& settings::environment() const
    {
        return _environment;
    }

    string const& settings::environment_directory() const
    {
        return _environment_directory;
    }

    vector<string> const& settings::module_directories() const
    {
        return _module_directories;
    }

    vector<string> const& settings::manifests() const
    {
        return _manifests;
    }

    string const& settings::node_name() const
    {
        return _node_name;
    }

    string const& settings::output_file() const
    {
        return _output_file;
    }

    string const& settings::graph_file() const
    {
        return _graph_file;
    }

    shared_ptr<facts::provider> const& settings::facts() const
    {
        return _facts;
    }

    logging::level settings::log_level() const
    {
        return _log_level;
    }

    bool settings::show_help() const
    {
        return _show_help;
    }

    bool settings::show_version() const
    {
        return _show_version;
    }

    void settings::print_usage()
    {
        cout <<
            "Synopsis\n"
            "========\n"
            "\n"
            "Compiles Puppet manifests into Puppet catalogs.\n"
            "\n"
            "Usage\n"
            "=====\n"
            "\n"
            "  puppetcpp [options] [[manifest] [manifest] ...]\n"
            "\n"
            "Options\n"
            "=======\n\n"
            << create_options() <<
            "\nDescription\n"
            "===========\n"
            "\n"
            "Compiles a Puppet manifest into a Puppet catalog. When invoked with no options,\n"
            "the compiler will compile the manifest for the 'production' environment.\n"
            "\n"
            "Manifests will be evaluated in the order they are presented on the command line.\n"
            "\n"
            "Examples\n"
            "========\n\n"
            "  puppetcpp\n"
            "  puppetcpp manifest.pp\n"
            "  puppetcpp -e test -f facts.yaml"
            << endl;
    }

    void settings::parse(int argc, char const* argv[])
    {
        po::variables_map vm;

        auto visible_options = create_options();

        // Build a list of "hidden" options that are not visible on the command line
        po::options_description hidden_options("");
        hidden_options.add_options()
            ("manifests", po::value<vector<string>>());

        // Create the supported command line options (visible + hidden)
        po::options_description command_line_options;
        command_line_options.add(visible_options).add(hidden_options);

        // Build a list of positional options (in our case, just manifest files)
        po::positional_options_description positional_options;
        positional_options.add("manifests", -1);

        try {
            // Store the options
            po::store(
                po::command_line_parser(argc, argv).
                    style(po::command_line_style::unix_style & ~po::command_line_style::allow_guessing).
                    options(command_line_options).
                    positional(positional_options).
                    run(), vm);

            // Check for help or version before notifying
            if (vm.count("help")) {
                _show_help = true;
                return;
            }
            if (vm.count("version")) {
                _show_version = true;
                return;
            }

            // Notify the callbacks
            po::notify(vm);
        } catch (po::error const& ex) {
            throw settings_exception(ex.what());
        } catch (runtime_error const& ex) {
            throw settings_exception(ex.what());
        }

        // Populate the logging level
        _log_level = get_level(vm);

        // Populate the code directory
        _code_directory = get_code_directory(vm);

        // Populate the environment
        _environment = get_environment(vm);

        // Populate the environment's directory
        _environment_directory = get_environment_directory(vm, _code_directory, _environment);

        // Populate the module directories
        _module_directories = get_module_directories(vm, _code_directory);

        // Populate the facts provider
        _facts = get_facts(vm);

        // Populate the output file
        _output_file = get_output_file(vm);

        // Populate the graph file
        _graph_file = get_graph_file(vm);

        // Populate the node name
        _node_name = get_node(vm, *_facts);

        // Populate the manifests to compile
        _manifests = get_manifests(vm);
    }

}}  // namespace puppet::compiler