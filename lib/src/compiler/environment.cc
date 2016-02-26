#include <puppet/compiler/environment.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/logging/logger.hpp>
#include <puppet/utility/filesystem/helpers.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::utility::filesystem;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace compiler {

    environment::environment(string name, string directory) :
        finder(rvalue_cast(directory)),
        _name(rvalue_cast(name))
    {
        // Add the built-in functions and operators to the dispatcher
        _dispatcher.add_builtins();
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

    void environment::load(
        logging::logger& logger,
        string const& code_directory,
        string const& base_module_path,
        vector<string> const& manifests
    )
    {
        // TODO: load settings from environment.conf (the below should be the defaults)
        string module_path = string("modules") + path_separator() + base_module_path;
        string main_manifest = "manifests";

        // Replace base module path and code dir settings in the module path
        boost::replace_all(module_path, "$basemodulepath", base_module_path);
        boost::replace_all(module_path, "$codedir", code_directory);
        LOG(debug, "searching for modules using '%1%' for the module path.", module_path);

        // Go through each module directory to load modules
        boost::split_iterator<string::iterator> end;
        for (auto it = boost::make_split_iterator(module_path, boost::first_finder(path_separator(), boost::is_equal())); it != end; ++it) {
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

            load_modules(logger, path);
        }

        // If given manifests, use those; otherwise, fall back to the main manifest setting
        if (!manifests.empty()) {
            for (auto const& manifest : manifests) {
                auto path = make_absolute(manifest);

                sys::error_code ec;
                if (fs::is_regular_file(path, ec)) {
                    _manifests.emplace_back(rvalue_cast(path));
                    continue;
                }

                add_manifests(logger, path, true);
            }
        } else {
            auto path = make_absolute(main_manifest, this->directory());

            sys::error_code ec;
            if (fs::is_regular_file(path, ec)) {
                LOG(debug, "using '%1%' as the main manifest.", path);
                _manifests.emplace_back(rvalue_cast(path));
            } else {
                ec.clear();
                if (fs::is_directory(path, ec)) {
                    LOG(debug, "searching '%1%' for main manifests.", path);
                    add_manifests(logger, path);
                }
            }
        }

        if (_manifests.empty()) {
            throw compilation_exception(
                (boost::format("no manifests were found for environment '%1%': expected at least one manifest as an argument.") %
                 name()
                ).str()
            );
        }
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
        if (!fs::is_directory(directory, ec)) {
            LOG(debug, "skipping module directory '%1%' because it is not a directory.", directory);
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
