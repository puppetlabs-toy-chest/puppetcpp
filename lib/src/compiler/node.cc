#include <puppet/compiler/node.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime;
using namespace puppet::logging;

namespace puppet { namespace compiler {

    compilation_exception::compilation_exception(string const& message, string path, size_t line, size_t column, string text) :
        runtime_error(message),
        _path(rvalue_cast(path)),
        _line(line),
        _column(column),
        _text(rvalue_cast(text))
    {
    }

    string const& compilation_exception::path() const
    {
        return _path;
    }

    size_t compilation_exception::line() const
    {
        return _line;
    }

    size_t compilation_exception::column() const
    {
        return _column;
    }

    string const& compilation_exception::text() const
    {
        return _text;
    }

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

        // TODO: add support for Puppet's node_name (cert vs. facter) setting?
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

    catalog node::compile(logging::logger& logger, string const& path)
    {
        auto compilation_context = make_shared<compiler::context>(logger, make_shared<string>(path), *this);

        try {
            runtime::catalog catalog;
            runtime::context evaluation_context{ &catalog };

            // TODO: set parameters and facts in the top scope

            // TODO: create settings scope in catalog

            // Evaluate the syntax tree
            logger.log(level::debug, "evaluating the syntax tree for parsed file '%1%'.", path);
            expression_evaluator evaluator{compilation_context, evaluation_context};
            evaluator.evaluate();

            // Evaluate the node context
            logger.log(level::debug, "evaluating context for node '%1%'.", name());
            if (!catalog.evaluate_node(evaluation_context, *this)) {
                throw compilation_exception((boost::format("failed to evaluate node definition for node '%1%'.") % name()).str(), path);
            }

            // TODO: evaluate node classes

            // TODO: evaluate generators

            // TODO: finalize catalog
            return catalog;
        } catch (evaluation_exception const& ex) {
            throw compilation_context->create_exception(ex.position(), ex.what());
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

}}  // namespace puppet::compiler
