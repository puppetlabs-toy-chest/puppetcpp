#include <puppet/compiler/environment.hpp>
#include <puppet/compiler/parser/parser.hpp>
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

    shared_ptr<environment> environment::create(logging::logger& logger, compiler::settings settings, vector<string> const& manifests)
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

        auto environment = make_shared<make_shared_enabler>(rvalue_cast(name), rvalue_cast(base_directory), rvalue_cast(settings));
        environment->populate(logger, manifests);
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

    vector<string> const& environment::manifests() const
    {
        return _manifests;
    }

    void environment::compile(evaluation::context& context)
    {
        auto& logger = context.node().logger();

        // Load all the main manifests
        vector<shared_ptr<ast::syntax_tree>> trees;
        for (auto const& manifest : _manifests) {
            try {
                trees.emplace_back(import(logger, manifest));
            } catch (parse_exception const& ex) {
                throw compilation_exception(ex, manifest);
            }
        }

        evaluation::evaluator evaluator{ context };

        // Now evaluate the parsed syntax trees
        for (auto const& tree : trees) {
            LOG(debug, "evaluating the syntax tree for '%1%'.", tree->path());
            evaluator.evaluate(*tree);
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
                result.first->expression());
            if (!resource) {
                throw evaluation_exception("failed to add node resource.");
            }

            LOG(debug, "evaluating node definition for node '%1%'.", name());
            result.first->evaluate(context, *resource);
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

    void environment::import(logging::logger& logger, find_type type, string const& name)
    {
        string path;
        compiler::module const* module = nullptr;
        auto pos = name.find("::");
        if (pos == string::npos) {
            // If not a manifest, the name could not be imported
            if (type != find_type::manifest) {
                return;
            }

            // If the name is that of a module, translate to the init.pp manifest file
            module = find_module(name);
            if (!module) {
                LOG(debug, "could not load 'init.pp' for module '%1%' because the module does not exist.", name);
                return;
            }
            path = module->find(type, "init");
        } else {
            // Split into module name and subpath
            auto module_name = name.substr(0, pos);
            auto qualified_name = name.substr(pos + 2);
            module = find_module(module_name);
            if (!module) {
                LOG(debug, "could not load a file for '%1%' because module '%2%' does not exist.", name, module_name);
                return;
            }
            path = module->find(type, qualified_name);
        }

        if (path.empty()) {
            return;
        }

        // Ignore files that don't exist
        sys::error_code ec;
        if (!fs::is_regular_file(path, ec)) {
            return;
        }

        // Import the file, but don't parse it if it's already been imported
        import(logger, path, module);
    }

    environment::environment(string name, string directory, compiler::settings settings) :
        finder(rvalue_cast(directory)),
        _name(rvalue_cast(name)),
        _settings(rvalue_cast(settings))
    {
    }

    void environment::populate(logging::logger& logger, vector<string> const& manifests)
    {
        load_environment_settings(logger);

        add_modules(logger);

        if (manifests.empty()) {
            add_manifests(logger);
        } else {
            for (auto const& manifest : manifests) {
                auto path = make_absolute(manifest);

                sys::error_code ec;
                if (fs::is_regular_file(path, ec)) {
                    _manifests.emplace_back(rvalue_cast(path));
                    continue;
                }

                add_manifests(logger, path, true);
            }
        }
    }

    void environment::load_environment_settings(logging::logger& logger)
    {
        static char const* const CONFIGURATION_FILE = "environment.conf";

        string config_file_path = (fs::path{ this->directory() } / CONFIGURATION_FILE).string();
        sys::error_code ec;
        if (!fs::is_regular_file(config_file_path, ec)) {
            LOG(debug, "environment configuration file '%1%' was not found; using defaults.", config_file_path);
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
                _settings.set(settings::module_path, rvalue_cast(module_path));
            }
            if (vm.count(settings::manifest)) {
                auto manifest = vm[settings::manifest].as<string>();
                LOG(debug, "using main manifest '%1%' from environment configuration file.", manifest);
                _settings.set(settings::manifest, rvalue_cast(manifest));
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

    void environment::add_modules(logging::logger& logger)
    {
        auto module_path = _settings.get(settings::module_path);
        if (!module_path.as<string>()) {
            throw compilation_exception(
                (boost::format("expected a string for $%1% setting.") %
                 settings::module_path
                ).str()
            );
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

    void environment::add_manifests(logging::logger& logger)
    {
        auto main_manifest = _settings.get(settings::manifest);
        if (!main_manifest.as<string>()) {
            throw compilation_exception(
                (boost::format("expected a string for $%1% setting.") %
                 settings::manifest
                ).str()
            );
        }

        auto path = make_absolute(*main_manifest.as<string>(), this->directory());

        sys::error_code ec;
        if (fs::is_regular_file(path, ec)) {
            LOG(debug, "using '%1%' as the main manifest.", path);
            _manifests.emplace_back(rvalue_cast(path));
            return;
        }

        ec.clear();
        if (fs::is_directory(path, ec)) {
            add_manifests(logger, path);
        }
    }

    void environment::add_manifests(logging::logger& logger, string const& directory, bool throw_if_missing)
    {
        sys::error_code ec;
        if (!fs::is_directory(directory, ec) || ec) {
            if (throw_if_missing) {
                throw compilation_exception((boost::format("'%1%' is not a manifest file or a directory containing manifests.") % directory).str());
            }
            LOG(debug, "skipping manifest directory '%1%' because it is not a directory.", directory);
            return;
        }
        LOG(debug, "searching for manifests in '%1%'.", directory);

        fs::directory_iterator it{ directory };
        fs::directory_iterator end{};

        // Add the files to a vector so we can sort them first
        vector<string> manifests;
        for (; it != end; ++it) {
            if (fs::is_regular_file(it->status()) && it->path().extension() == ".pp") {
                manifests.emplace_back(it->path().string());
            }
        }

        // Sort the paths so they are in a deterministic order
        sort(manifests.begin(), manifests.end());

        for (auto& manifest : manifests) {
            LOG(debug, "found main manifest '%1%'.", manifest);
            _manifests.emplace_back(rvalue_cast(manifest));
        }
    }

    shared_ptr<ast::syntax_tree> environment::import(logging::logger& logger, string const& path, compiler::module const* module)
    {
        // TODO: this needs to be made thread safe

        try {
            shared_ptr<ast::syntax_tree> tree;

            // Check for a already parsed AST
            auto it = _parsed.find(path);
            if (it != _parsed.end()) {
                LOG(debug, "using cached AST for '%1%' in environment '%2%'.", path, name());
                tree = it->second;
            } else {
                // Parse the file
                LOG(debug, "loading '%1%' into environment '%2%'.", path, name());
                tree = parser::parse_file(logger, path, module);
                LOG(debug, "parsed AST for '%1%':\n-----\n%2%\n-----", path, *tree);
                _parsed.emplace(path, tree);
            }
            _registry.import(*tree);
            return tree;
        } catch (parse_exception const& ex) {
            throw compilation_exception(ex, path);
        }
    }

}}  // namespace puppet::compiler
