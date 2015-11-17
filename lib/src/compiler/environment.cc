#include <puppet/compiler/environment.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/logging/logger.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace puppet::runtime;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace compiler {

    environment::environment(logging::logger& logger, compiler::settings const& settings, string name, string directory) :
        finder(rvalue_cast(directory)),
        _settings(settings),
        _name(rvalue_cast(name))
    {
        // First load this module's directories
        // TODO: the modules subdirectory can come from an environment configuration file
        load_modules(logger, (fs::path{ this->directory() } / "modules").string());

        // Next load global modules
        for (auto const& directory : _settings.module_directories()) {
            load_modules(logger, directory);
        }
    }

    compiler::settings const& environment::settings() const
    {
        return _settings;
    }

    string const& environment::name() const
    {
        return _name;
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

    void environment::compile(evaluation::context& context)
    {
        auto& logger = context.node().logger();

        // If files to compile were explicitly specified, load those files only
        vector<shared_ptr<ast::syntax_tree>> trees;
        if (!_settings.manifests().empty()) {
            // Treat the files as if they come from the environment
            for (auto const& manifest : _settings.manifests()) {
                try {
                    trees.emplace_back(import(logger, manifest));
                } catch (parse_exception const& ex) {
                    throw compilation_exception(ex, manifest);
                }
            }
        } else {
            // TODO: the "manifests" directory is a configuration setting; should not be hard coded
            auto manifests_directory = fs::path{ directory() } / "manifests";

            sys::error_code ec;
            if (!fs::is_directory(manifests_directory, ec) || ec) {
                // Base directory doesn't exist
                LOG(debug, "manifest directory does not exist '%1%'.", manifests_directory.string());
            } else {
                LOG(debug, "loading manifests in '%1%'.", manifests_directory.string());

                fs::directory_iterator it{manifests_directory};
                fs::directory_iterator end{};

                // Add the files to a vector
                vector<string> manifests;
                for (; it != end; ++it) {
                    if (fs::is_regular_file(it->status()) && it->path().extension() == ".pp") {
                        manifests.emplace_back(it->path().string());
                    }
                }

                // Sort the paths so they are in a deterministic order
                sort(manifests.begin(), manifests.end());

                // Load the manifests
                for (auto const& manifest : manifests) {
                    try {
                        trees.emplace_back(import(logger, manifest));
                    } catch (parse_exception const& ex) {
                        throw compilation_exception(ex, manifest);
                    }
                }
            }
        }

        if (trees.empty()) {
            throw compiler::compilation_exception(
                (boost::format("no manifests were found for environment '%1%': expected at least one manifest to compile as an argument.") %
                 _name
                ).str());
        }

        evaluation::evaluator evaluator{ context };

        // Now evaluate the parsed syntax trees
        for (auto const& tree : trees) {
            LOG(debug, "evaluating the syntax tree for '%1%'.", tree->path());
            evaluator.evaluate(*tree);
        }

        // Find and evaluate a node definition
        auto result = _registry.find_node(context.node());
        if (result.first) {
            auto& catalog = context.catalog();
            auto resource = catalog.add(
                types::resource("node", result.second),
                catalog.find(types::resource("class", "main")),
                &result.first->expression().context);
            if (!resource) {
                throw evaluation_exception("failed to add node resource.");
            }

            LOG(debug, "evaluating node definition for node '%1%'.", name());
            result.first->evaluate(context, *resource);
        }
    }

    module* environment::find_module(string const& name)
    {
        // TODO: this function needs to be thread safe

        auto it = _modules.find(name);
        if (it == _modules.end()) {
            return nullptr;
        }
        return &it->second;
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

    void environment::load_modules(logging::logger& logger, string const& directory)
    {
        sys::error_code ec;
        if (!fs::is_directory(directory, ec) || ec) {
            LOG(debug, "modules directory '%1%' does not exist.", directory);
            return;
        }

        fs::directory_iterator it{directory};
        fs::directory_iterator end{};

        // Search for modules
        LOG(debug, "searching '%1%' for modules.", directory);
        for (; it != end; ++it) {
            // If not a directory, ignore
            if (!fs::is_directory(it->status())) {
                continue;
            }
            auto const& module_directory = it->path().string();
            auto name = it->path().filename().string();
            if (name == "lib") {
                // Warn that the module path may not be set correctly, but add a "lib" module
                LOG(warning, "found module named 'lib' at '%1%': this may indicate the module search path is incorrect.", module_directory);
            }  else if (!module::is_valid_name(name)) {
                // Warn about an invalid name
                LOG(warning, "found module with invalid name '%1%' at '%2%': module will be ignored.", name, module_directory);
                continue;
            }

            auto existing = find_module(name);
            if (existing) {
                LOG(warning, "module '%1%' at '%2%' conflicts with existing module at '%3%' and will be ignored.", name, module_directory, existing->directory());
                continue;
            }

            LOG(debug, "found module '%1%' at '%2%'.", name, module_directory);
            module mod{ *this, module_directory, name };
            _modules.emplace(rvalue_cast(name), rvalue_cast(mod));
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
                LOG(debug, "using cached AST for '%1%' in environment '%2%'.", path, _name);
                tree = it->second;
            } else {
                // Parse the file
                LOG(debug, "loading '%1%' into environment '%2%'.", path, _name);
                tree = parser::parse_file(path, module);
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
