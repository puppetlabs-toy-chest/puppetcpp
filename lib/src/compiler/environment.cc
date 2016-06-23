#include <puppet/compiler/environment.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/scanner.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/logging/logger.hpp>
#include <puppet/utility/filesystem/helpers.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::utility::filesystem;
namespace fs = boost::filesystem;
namespace sys = boost::system;
namespace po = boost::program_options;

namespace puppet { namespace compiler {

    static void load_environment_settings(logging::logger& logger, string const& directory, compiler::settings& settings)
    {
        static char const* const CONFIGURATION_FILE = "environment.conf";

        string config_file_path = (fs::path{ directory } / CONFIGURATION_FILE).string();
        sys::error_code ec;
        if (!fs::is_regular_file(config_file_path, ec)) {
            LOG(debug, "environment configuration file '%1%' was not found.", config_file_path);
            return;
        }

        LOG(debug, "loading environment settings from '%1%'.", config_file_path);

        try {
            // Read the options from the config file
            po::options_description description("");
            description.add_options()
                (settings::module_path.c_str(), po::value<string>(), "")
                (settings::manifest.c_str(),    po::value<string>(), "");
            po::variables_map vm;
            po::store(po::parse_config_file<char>(config_file_path.c_str(), description, true), vm);
            po::notify(vm);

            if (vm.count(settings::module_path)) {
                auto module_path = vm[settings::module_path].as<string>();
                LOG(debug, "using module path '%1%' from environment configuration file.", module_path);
                settings.set(settings::module_path, rvalue_cast(module_path));
            }
            if (vm.count(settings::manifest)) {
                auto manifest = vm[settings::manifest].as<string>();
                LOG(debug, "using main manifest '%1%' from environment configuration file.", manifest);
                settings.set(settings::manifest, rvalue_cast(manifest));
            }
        } catch (po::error const& ex) {
            throw compilation_exception(
                (boost::format("failed to read environment configuration file '%1%': %2%.") %
                 config_file_path %
                 ex.what()
                ).str()
            );
        }
    }

    shared_ptr<environment> environment::create(logging::logger& logger, compiler::settings settings)
    {
        // Get the name from the settings
        string name = boost::lexical_cast<string>(settings.get(settings::environment, false));
        if (name.empty()) {
            throw compilation_exception("cannot create an environment with an empty name.");
        }

        // Search for the environment's directory
        string base_directory;
        auto environment_path = settings.get(settings::environment_path);
        LOG(debug, "searching for environment '%1%' using environment path '%2%'.", name, environment_path);
        if (environment_path.as<string>()) {
            boost::split_iterator<string::const_iterator> end;
            for (auto it = boost::make_split_iterator(*environment_path.as<string>(), boost::first_finder(path_separator(), boost::is_equal())); it != end; ++it) {
                if (!*it) {
                    continue;
                }

                auto path = fs::path{ make_absolute({ it->begin(), it->end() }) } / name;

                sys::error_code ec;
                if (fs::is_directory(path, ec)) {
                    base_directory = path.string();
                    break;
                }
            }
        }
        if (base_directory.empty()) {
            throw compilation_exception(
                (boost::format("could not locate an environment directory for environment '%1%' using search path '%2%'.") %
                 name %
                 environment_path
                ).str());
        }

        LOG(debug, "found environment directory '%1%' for environment '%2%'.", base_directory, name);

        struct make_shared_enabler : environment
        {
            explicit make_shared_enabler(string name, string base, compiler::settings settings) :
                environment(rvalue_cast(name), rvalue_cast(base), rvalue_cast(settings))
            {
            }
        };

        // Load the environment settings
        load_environment_settings(logger, base_directory, settings);

        auto environment = make_shared<make_shared_enabler>(rvalue_cast(name), rvalue_cast(base_directory), rvalue_cast(settings));
        environment->add_modules(logger);
        return environment;
    }

    string const& environment::name() const
    {
        return _name;
    }

    compiler::settings const& environment::settings() const
    {
        return _settings;
    }

    compiler::registry& environment::registry()
    {
        return _registry;
    }

    compiler::registry const& environment::registry() const
    {
        return _registry;
    }

    evaluation::dispatcher& environment::dispatcher()
    {
        return _dispatcher;
    }

    evaluation::dispatcher const& environment::dispatcher() const
    {
        return _dispatcher;
    }

    deque<module> const& environment::modules() const
    {
        return _modules;
    }

    void environment::compile(evaluation::context& context, vector<string> const& manifests)
    {
        auto& logger = context.node().logger();

        vector<shared_ptr<ast::syntax_tree>> trees;
        auto parse = [&](string const& manifest) {
            try {
                trees.emplace_back(import(logger, manifest));
            } catch (parse_exception const& ex) {
                throw compilation_exception(ex, manifest);
            }
        };

        // Load all the main manifests
        if (manifests.empty()) {
            each_file(find_type::manifest, [&](auto const& manifest) {
                parse(manifest);
                return true;
            });
        } else {
            // Set the finder to treat the manifests base as the manifest itself
            // This handles recursively searching for manifests if a directory
            compiler::settings temp;
            temp.set(settings::manifest, ".");

            for (auto& manifest : manifests) {
                compiler::finder finder{ manifest, &temp };
                finder.each_file(find_type::manifest, [&](auto const& manifest) {
                    parse(manifest);
                    return true;
                });
            }
        }

        {
            // Create the 'main' stack frame
            evaluation::scoped_stack_frame frame{ context, evaluation::stack_frame{ "<class main>", context.top_scope(), false }};
            evaluation::evaluator evaluator{ context };

            // Now evaluate the parsed syntax trees
            for (auto const& tree : trees) {
                LOG(debug, "evaluating the syntax tree for '%1%'.", tree->path());
                evaluator.evaluate(*tree);
            }
        }

        // Find and evaluate a node definition
        if (_registry.has_nodes()) {
            auto result = _registry.find_node(context.node());
            if (!result.first) {
                ostringstream message;
                message << "could not find a default node or a node with the following names: ";
                bool first = true;
                context.node().each_name([&](string const& name) {
                    if (first) {
                        first = false;
                    } else {
                        message << ", ";
                    }
                    message << name;
                    return true;
                });
                 message << ".";
                throw compiler::compilation_exception(message.str());
            }

            auto& catalog = context.catalog();
            auto resource = catalog.add(
                types::resource("node", result.second),
                catalog.find(types::resource("class", "main")),
                context.top_scope(),
                result.first->statement());
            if (!resource) {
                throw evaluation_exception("failed to add node resource.", context.backtrace());
            }

            LOG(debug, "evaluating node definition for node '%1%'.", context.node().name());
            evaluation::node_evaluator evaluator{ context, result.first->statement() };
            evaluator.evaluate(*resource);
        }
    }

    module* environment::find_module(string const& name)
    {
        return const_cast<module*>(static_cast<environment const*>(this)->find_module(name));
    }

    module const* environment::find_module(string const& name) const
    {
        auto it = _module_map.find(name);
        if (it == _module_map.end()) {
            return nullptr;
        }
        return it->second;
    }

    void environment::each_module(function<bool(module const&)> const& callback) const
    {
        // TODO: this function needs to be thread safe
        for (auto& module : _modules) {
            if (!callback(module)) {
                return;
            }
        }
    }

    void environment::import(logging::logger& logger, find_type type, string name)
    {
        boost::to_lower(name);
        if (boost::starts_with(name, "::")) {
            name = name.substr(2);
        }

        string path;
        compiler::module const* module = nullptr;
        auto pos = name.find("::");
        if (pos == string::npos) {
            // If not a manifest, the name could not be imported
            if (type != find_type::manifest) {
                return;
            }

            if (name == "environment") {
                // Don't load manifests from the environment
                return;
            }

            // If the name is that of a module, translate to the init.pp manifest file
            module = find_module(name);
            if (!module) {
                LOG(debug, "could not load 'init.pp' for module '%1%' because the module does not exist.", name);
                return;
            }
            path = module->find_file(type, "init");
        } else {
            // Split into namespace and subname
            auto ns = name.substr(0, pos);
            auto subname = name.substr(pos + 2);

            if (ns == "environment") {
                if (type == find_type::manifest) {
                    // Don't load manifests from the environment
                    return;
                }
                path = this->find_file(type, subname);
            } else {
                module = find_module(ns);
                if (!module) {
                    LOG(debug, "could not load a file for '%1%' because module '%2%' does not exist.", name, ns);
                    return;
                }
                path = module->find_file(type, subname);
            }
        }

        // Ignore files that don't exist
        if (path.empty()) {
            return;
        }

        // Import the file, but don't parse it if it's already been imported
        import(logger, path, module);
    }

    environment::environment(string name, string directory, compiler::settings settings) :
        finder(rvalue_cast(directory), &settings),
        _name(rvalue_cast(name)),
        _settings(rvalue_cast(settings))
    {
    }

    void environment::add_modules(logging::logger& logger)
    {
        auto module_path = _settings.get(settings::module_path);
        if (!module_path.as<string>()) {
            throw compilation_exception((boost::format("expected a string for $%1% setting.") % settings::module_path).str());
        }

        LOG(debug, "searching for modules using module path '%1%'.", module_path);

        // Go through each module directory to load modules
        boost::split_iterator<string::const_iterator> end;
        for (auto it = boost::make_split_iterator(*module_path.as<string>(), boost::first_finder(path_separator(), boost::is_equal())); it != end; ++it) {
            if (!*it) {
                continue;
            }

            string directory{ it->begin(), it->end() };
            auto path = make_absolute(directory, this->directory());

            sys::error_code ec;
            if (!fs::is_directory(path, ec)) {
                LOG(debug, "skipping module directory '%1%' because it is not a directory.", path);
                continue;
            }

            add_modules(logger, path);
        }
    }

    void environment::add_modules(logging::logger& logger, string const& directory)
    {
        sys::error_code ec;
        if (!fs::is_directory(directory, ec)) {
            LOG(debug, "skipping module directory '%1%' because it is not a directory.", directory);
            return;
        }

        fs::directory_iterator it{directory};
        fs::directory_iterator end{};

        // Search for modules
        vector<pair<string, string>> modules;
        LOG(debug, "searching '%1%' for modules.", directory);
        for (; it != end; ++it) {
            // If not a directory, ignore
            if (!fs::is_directory(it->status())) {
                continue;
            }
            modules.emplace_back(it->path().string(), it->path().filename().string());
        }

        // Sort the directories to ensure a deterministic order
        sort(modules.begin(), modules.end(), [](auto const& left, auto const& right) { return left.second < right.second; });

        for (auto& module : modules) {
            if (module.second == "lib") {
                // Warn that the module path may not be set correctly, but add a "lib" module
                LOG(warning, "found module named 'lib' at '%1%': this may indicate the module search path is incorrect.", module.first);
            }  else if (!module::is_valid_name(module.second)) {
                // Warn about an invalid name
                LOG(warning, "found module with invalid name '%1%' at '%2%': module will be ignored.", module.second, module.first);
                continue;
            }

            auto existing = find_module(module.second);
            if (existing) {
                LOG(warning, "module '%1%' at '%2%' conflicts with existing module at '%3%' and will be ignored.", module.second, module.first, existing->directory());
                continue;
            }

            LOG(debug, "found module '%1%' at '%2%'.", module.second, module.first);
            _modules.emplace_back(*this, rvalue_cast(module.first), rvalue_cast(module.second));
            _module_map.emplace(_modules.back().name(), &_modules.back());
        }
    }

    shared_ptr<ast::syntax_tree> environment::import(logging::logger& logger, string const& path, compiler::module const* module)
    {
        // TODO: this needs to be made thread safe
        // TODO: this needs to be made transactional

        try {
            // Check for an already parsed AST
            auto it = _parsed.find(path);
            if (it != _parsed.end()) {
                LOG(debug, "using cached AST for '%1%' in environment '%2%'.", path, name());
                return it->second;
            }

            // Parse the file
            LOG(debug, "loading '%1%' into environment '%2%'.", path, name());
            auto tree = parser::parse_file(logger, path, module);
            LOG(debug, "parsed AST for '%1%':\n-----\n%2%\n-----", path, *tree);
            _parsed.emplace(path, tree);

            // Validate the AST
            tree->validate();

            // Scan the tree for definitions
            compiler::scanner scanner{ _registry, _dispatcher };
            scanner.scan(*tree);
            return tree;
        } catch (parse_exception const& ex) {
            throw compilation_exception(ex, path);
        }
    }

}}  // namespace puppet::compiler
