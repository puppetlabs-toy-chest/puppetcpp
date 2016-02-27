#include <puppet/compiler/node.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/evaluation/context.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::logging;

namespace puppet { namespace compiler {

    node::node(logging::logger& logger, string const& name, shared_ptr<compiler::environment> environment, shared_ptr<facts::provider> facts) :
        _logger(logger),
        _environment(rvalue_cast(environment)),
        _facts(rvalue_cast(facts))
    {
        if (!_environment) {
            throw runtime_error("expected an environment for the node.");
        }

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

    logging::logger& node::logger()
    {
        return _logger;
    }

    string const& node::name() const
    {
        // Return the last name in the set, which is always the most specific
        return *_names.rbegin();
    }

    compiler::environment& node::environment() const
    {
        return *_environment;
    }

    shared_ptr<facts::provider> const& node::facts() const
    {
        return _facts;
    }

    catalog node::compile()
    {
        try {
            compiler::catalog catalog{ name(), _environment->name() };

            // Create the evaluation context
            evaluation::context context{ *this, catalog };
            create_initial_resources(context);

            // TODO: set node parameters in the top scope

            // Compile the associated environment
            _environment->compile(context);

            // TODO: evaluate node classes

            // Finalize the evaluation context
            context.finalize();

            // Populate relationship metaparameters to the graph
            catalog.populate_graph();
            return catalog;
        } catch (evaluation_exception const& ex) {
            throw compilation_exception(ex);
        }
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

    void node::create_initial_resources(evaluation::context& context) const
    {
        auto& catalog = context.catalog();

        // Create Stage[main]
        auto main_stage = catalog.add(types::resource("stage", "main"));
        if (!main_stage) {
            throw runtime_error("expected main stage to not be present.");
        }

        // Create Class[settings]
        auto settings = catalog.add(types::resource("class", "settings"), main_stage);
        if (!settings) {
            throw runtime_error("expected settings class to not be present.");
        }
        auto scope = make_shared<evaluation::scope>(context.top_scope(), settings);
        context.add_scope(scope);

        // Set the settings in the settings scope
        ast::context none;
        _environment->settings().each([&](string const& name, values::value const& value) {
            scope->set(name, std::make_shared<values::value>(value), none);
            return true;
        });

        // TODO: set settings in the scope

        auto main_class = catalog.add(types::resource("class", "main"), main_stage);
        if (!main_class) {
            throw runtime_error("expected main class to not be present.");
        }
        context.top_scope()->resource(main_class);
    }

}}  // namespace puppet::compiler
