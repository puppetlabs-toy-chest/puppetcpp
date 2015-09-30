#include <puppet/compiler/node.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/runtime/definition_scanner.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime;
using namespace puppet::logging;

namespace puppet { namespace compiler {

    node::node(
        logging::logger& logger,
        string const& name,
        vector<string> const& module_directories,
        compiler::environment& environment) :
        _logger(logger),
        _environment(environment)
    {
        // Copy each subname of the node name
        // For example, a node name of 'foo.bar.baz' would emplace 'foo', then 'foo.bar', then 'foo.bar.baz'.
        boost::split_iterator<string::const_iterator> end;
        for (auto it = boost::make_split_iterator(name, boost::first_finder(".", boost::is_equal())); it != end; ++it) {
            if (!*it) {
                continue;
            }
            string hostname(name.begin(), it->end());
            boost::to_lower(hostname);
            _names.emplace(rvalue_cast(hostname));
        }

        _modules = load_modules(_logger, module_directories);
    }

    logging::logger& node::logger()
    {
        return _logger;
    }

    string const& node::name() const
    {
        // Return the last name in the set, which is always the most specific
        return *_names.rbegin();
    }

    compiler::environment const& node::environment() const
    {
        return _environment;
    }

    catalog node::compile(compiler::settings const& settings)
    {
        // Needed for LOG macro
        auto& logger = _logger;

        // Create a catalog, main evaluation context, and main compilation context
        runtime::catalog catalog;
        runtime::context evaluation_context{ *this, settings.facts(), &catalog };
        auto compilation_context = std::make_shared<compiler::context>(std::make_shared<string>("main"), *this, false /* dummy context */);

        // Get the initial manifests to compile
        auto manifests = settings.manifests();
        if (manifests.empty()) {
            manifests = _environment.find_manifests();
            if (manifests.empty()) {
                throw compiler::compilation_exception(
                        (boost::format("no manifests were found for environment '%1%': expected at least one manifest to compile as an argument.") %
                         _environment.name()
                        ).str());
            }
        }

        // Create an evaluation context and a settings scope
        create_initial_resources(evaluation_context, compilation_context, settings);

        // TODO: set node parameters in the top scope

        // First parse all the files so they can be scanned
        vector<shared_ptr<compiler::context>> contexts;
        contexts.reserve(manifests.size() + 1);

        try {
            definition_scanner scanner{ catalog };
            for (auto const& manifest : manifests) {
                // Create a compilation context for each manifest (parses the file)
                contexts.push_back(make_shared<compiler::context>(make_shared<string>(manifest), *this));

                // Scan this context for definitions
                scanner.scan(contexts.back());
            }

            // Now evaluate the manifests in the specified order
            for (auto const& context : contexts) {
                // Evaluate the syntax tree
                LOG(debug, "evaluating the syntax tree for '%1%'.", *context->path());
                expression_evaluator evaluator{context, evaluation_context};
                evaluator.evaluate();
            }

            // Evaluate the node definition
            LOG(debug, "evaluating node definition for node '%1%'.", name());
            catalog.declare_node(evaluation_context, *this);

            // TODO: evaluate node classes

            // TODO: evaluate generators

            // Finalize the catalog
            LOG(debug, "generating resources and populating dependency graph.");
            catalog.finalize(evaluation_context);
        } catch (evaluation_exception const& ex) {
            if (!ex.context()) {
                throw compilation_exception(ex.what());
            }
            throw ex.context()->create_exception(ex.position(), ex.what());
        }
        return catalog;
    }

    void node::each_name(function<bool(string const&)> const& callback) const
    {
        // Set goes from most specific to least specific in order, so traverse backwards
        for (auto it = _names.crbegin(); it != _names.crend(); ++it) {
            if (!callback(*it)) {
                return;
            }
        }
    }

    module const* node::find_module(string const& name) const
    {
        // First try the environment
        if (auto module = _environment.find_module(name)) {
            return module;
        }
        // Next try global modules
        auto it = _modules.find(name);
        if (it == _modules.end()) {
            return nullptr;
        }
        return &it->second;
    }

    void node::load_manifest(runtime::context& evaluation_context, string const& name)
    {
        auto& logger = _logger;

        // If already loaded, do nothing
        if (!_loaded_manifests.insert(name).second) {
            return;
        }

        compiler::module const* module = nullptr;
        auto pos = name.find("::");
        string manifest_name;
        if (pos == string::npos) {
            module = find_module(name);
            if (!module) {
                LOG(debug, "could not load initialization manifest for module '%1%' because the module does not exist.", name);
                return;
            }
        } else {
            auto module_name = name.substr(0, pos);
            manifest_name = name.substr(pos + 2);
            module = find_module(module_name);
            if (!module) {
                LOG(debug, "could not load a manifest for '%1%' because module '%2%' does not exist.", manifest_name, module_name);
                return;
            }
        }
        auto manifest = module->find_manifest(manifest_name);
        if (manifest.empty()) {
            if (manifest_name.empty()) {
                LOG(debug, "module '%1%' does not contain an initialization manifest.", module->name());
            } else {
                LOG(debug, "module '%1%' does not contain a manifest for '%2%'.", module->name(), manifest_name);
            }
            return;
        }

        auto compilation_context = make_shared<compiler::context>(make_shared<string>(rvalue_cast(manifest)), *this);
        if (evaluation_context.catalog()) {
            LOG(debug, "scanning for class or defined type definitions in '%1%'.", *compilation_context->path());
            definition_scanner scanner{ *evaluation_context.catalog() };
            scanner.scan(compilation_context);
        }
    }

    void node::create_initial_resources(
        runtime::context& evaluation_context,
        shared_ptr<compiler::context> const& compilation_context,
        compiler::settings const& settings)
    {
        auto catalog = evaluation_context.catalog();
        if (!catalog) {
            return;
        }

        // Use line 1 for the dummy position of these resources
        lexer::position position(0, 1);

        // Create Stage[main]
        auto& stage_main = catalog->add_resource(types::resource("stage", "main"), compilation_context, position);

        // Create Class[Settings] and add the settings scope
        auto& settings_resource = catalog->add_resource(types::resource("class", "settings"), compilation_context, position, &stage_main);
        auto settings_scope = make_shared<runtime::scope>(evaluation_context.top_scope(), &settings_resource);
        evaluation_context.add_scope(settings_scope);

        // TODO: set settings in the scope

        // Create Class[main] and associate it with the top scope
        auto& class_main = catalog->add_resource(types::resource("class", "main"), compilation_context, position, &stage_main);
        evaluation_context.top_scope()->resource(&class_main);
    }

}}  // namespace puppet::compiler
