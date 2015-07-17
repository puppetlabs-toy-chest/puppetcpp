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
        runtime::catalog catalog;
        runtime::context evaluation_context{ settings.facts(), &catalog };

        // TODO: set parameters and facts in the top scope

        // TODO: create settings scope in catalog

        // First parse all the files so they can be scanned
        vector<shared_ptr<compiler::context>> contexts;
        contexts.reserve(settings.manifests().size() + 1);

        for (auto const& manifest : settings.manifests()) {
            contexts.push_back(make_shared<compiler::context>(logger, make_shared<string>(manifest), *this));

            // Scan this context for definitions
            definition_scanner scanner{ catalog };
            scanner.scan(contexts.back());
        }

        // Now evaluate the manifests in the specified order
        for (auto const& context : contexts) {
            try {
                // Evaluate the syntax tree
                LOG(debug, "evaluating the syntax tree for '%1%'.", *context->path());
                expression_evaluator evaluator{context, evaluation_context};
                evaluator.evaluate();
            } catch (evaluation_exception const& ex) {
                throw ex.context()->create_exception(ex.position(), ex.what());
            }
        }

        // Evaluate the node definition
        LOG(debug, "evaluating node definition for node '%1%'.", name());
        if (!catalog.evaluate_node(evaluation_context, *this)) {
            throw compilation_exception((boost::format("failed to evaluate node definition for node '%1%'.") % name()).str());
        }

        // TODO: evaluate node classes

        // TODO: evaluate generators

        // Finalize the catalog
        catalog.finalize();

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

}}  // namespace puppet::compiler
