#include <puppet/compiler/node.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/runtime/definition_scanner.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime;
using namespace puppet::logging;

namespace puppet { namespace compiler {

    node::node(string const& name, compiler::environment& environment) :
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
    }

    string const& node::name() const
    {
        // Return the last name in the set, which is always the most specific
        return *_names.rbegin();
    }

    compiler::environment& node::environment()
    {
        return _environment;
    }

    catalog node::compile(logging::logger& logger, compiler::settings const& settings)
    {
        // Create a catalog, main evaluation context, and main compilation context
        runtime::catalog catalog;
        runtime::context evaluation_context{ settings.facts(), &catalog };
        auto compilation_context = std::make_shared<compiler::context>(logger, std::make_shared<string>("main"), *this, false /* dummy context */);

        // Create an evaluation context and a settings scope
        create_initial_resources(evaluation_context, compilation_context, settings);

        // TODO: set node parameters in the top scope

        // First parse all the files so they can be scanned
        vector<shared_ptr<compiler::context>> contexts;
        contexts.reserve(settings.manifests().size() + 1);

        try {
            definition_scanner scanner{ catalog };
            for (auto const& manifest : settings.manifests()) {
                // Create a compilation context for each manifest (parses the file)
                contexts.push_back(make_shared<compiler::context>(logger, make_shared<string>(manifest), *this));

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

        // Create Class[main] and associate it with the top scope
        auto& class_main = catalog->add_resource(types::resource("class", "main"), compilation_context, position, &stage_main);
        evaluation_context.top_scope()->resource(&class_main);

        // Contain Class[main] in Stage[main]
        catalog->add_relationship(relationship::contains, stage_main, class_main);

        // Create Class[Settings] and add the settings scope
        auto& settings_resource = catalog->add_resource(types::resource("class", "settings"), compilation_context, position, &stage_main);
        auto settings_scope = make_shared<runtime::scope>(evaluation_context.top_scope(), &settings_resource);
        evaluation_context.add_scope(settings_scope);

        // TODO: set settings in the scope

        // Contain Class[Settings] in Stage[main]
        catalog->add_relationship(relationship::contains, stage_main, settings_resource);
    }

}}  // namespace puppet::compiler
