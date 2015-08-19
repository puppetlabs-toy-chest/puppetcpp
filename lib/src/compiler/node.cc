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
        // Create a catalog with the "main" resources
        runtime::catalog catalog;
        create_main(catalog);

        // Create an evaluation context and a settings scope
        runtime::context evaluation_context{ settings.facts(), &catalog };
        create_settings_scope(evaluation_context, settings);

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
            catalog.finalize();
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

    void node::create_main(runtime::catalog& catalog)
    {
        auto path = make_shared<string>("<generated>");

        // Create Stage[main]
        catalog.add_resource(types::resource("stage", "main"), path, 1);

        // Create Class[main]
        catalog.add_resource(types::resource("class", "main"), path, 1);

        // TODO: add containment edge from Class[main] to Stage[main]
    }

    void node::create_settings_scope(runtime::context& context, compiler::settings const& settings)
    {
        auto catalog = context.catalog();
        if (!catalog) {
            return;
        }

        // Find Stage[main]
        auto main = catalog->find_resource(types::resource("stage", "main"));
        if (!main) {
            return;
        }

        // Create Class[Settings]
        auto& settings_resource = catalog->add_resource(types::resource("class", "settings"), main->path(), 1);
        auto settings_scope = make_shared<runtime::scope>(context.top_scope(), &settings_resource);
        context.add_scope(settings_scope);

        // TODO: set settings in the scope

        // TODO: add containment edge from Class[settings] -> Stage[main]
    }

}}  // namespace puppet::compiler
