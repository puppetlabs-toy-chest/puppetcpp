#include <puppet/runtime/context.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    match_scope::match_scope(runtime::context& context) :
        _context(context)
    {
        _context._match_stack.emplace_back();
    }

    match_scope::~match_scope()
    {
        _context._match_stack.pop_back();
    }

    local_scope::local_scope(runtime::context& context, shared_ptr<runtime::scope> scope) :
        match_scope(context),
        _context(context)
    {
        if (!scope) {
            scope = make_shared<runtime::scope>(_context.current_scope());
        }

        // Push the named scope onto the stack
        _context._scope_stack.emplace_back(rvalue_cast(scope));
    }

    local_scope::~local_scope()
    {
        _context._scope_stack.pop_back();
    }

    node_scope::node_scope(runtime::context& context, runtime::resource* resource) :
        _context(context)
    {
        // Create a node scope that inherits from the top scope
        _context._node_scope = make_shared<runtime::scope>(_context._scope_stack.front(), resource);
        _context._scope_stack.push_back(_context._node_scope);
    }

    node_scope::~node_scope()
    {
        _context._scope_stack.pop_back();
        _context._node_scope.reset();
    }

    context::context(shared_ptr<facts::provider> facts, runtime::catalog* catalog) :
        _catalog(catalog)
    {
        // Get the "main" resource if given a catalog
        runtime::resource* main = _catalog ? _catalog->find_resource(types::resource("class", "main")) : nullptr;

        // Add the top scope
        auto top = make_shared<runtime::scope>(rvalue_cast(facts), main);
        _scopes.emplace(make_pair("", top));
        _scope_stack.emplace_back(rvalue_cast(top));

        // Add an empty top match scope
        _match_stack.emplace_back();
    }

    runtime::catalog* context::catalog()
    {
        return _catalog;
    }

    shared_ptr<runtime::scope> const& context::current_scope()
    {
        return _scope_stack.back();
    }

    shared_ptr<runtime::scope> const& context::top_scope()
    {
        return _scope_stack.front();
    }

    shared_ptr<runtime::scope> const& context::node_scope()
    {
        return _node_scope;
    }

    shared_ptr<runtime::scope> const& context::node_or_top()
    {
        if (_node_scope) {
            return _node_scope;
        }
        return top_scope();
    }

    bool context::add_scope(std::shared_ptr<runtime::scope> scope)
    {
        if (!scope) {
            throw runtime_error("expected a non-null scope.");
        }
        if (!scope->resource()) {
            throw runtime_error("expected a scope with an associated resource.");
        }
        return _scopes.emplace(make_pair(scope->resource()->type().title(), rvalue_cast(scope))).second;
    }

    std::shared_ptr<runtime::scope> context::find_scope(string const& name) const
    {
        auto it = _scopes.find(name);
        if (it == _scopes.end()) {
            return nullptr;
        }
        return it->second;
    }

    void context::set(smatch const& matches)
    {
        if (_match_stack.empty()) {
            return;
        }

        auto& scope = _match_stack.back();

        // If there is no scope or a closure has captured the matches, reset
        if (!scope || !scope.unique()) {
            scope = make_shared<vector<shared_ptr<value const>>>();
        }

        scope->clear();
        scope->reserve(matches.size());

        for (auto const& match : matches) {
            scope->emplace_back(make_shared<value const>(match.str()));
        }
    }

    shared_ptr<value const> context::lookup(string const& name, expression_evaluator* evaluator, lexer::position const* position)
    {
        // Look for the last :: delimiter; if not found, use the current scope
        auto pos = name.rfind("::");
        if (pos == string::npos) {
            auto variable = current_scope()->get(name);
            return variable ? variable->value() : nullptr;
        }

        // Split into namespace and variable name
        // For global names, remove the leading ::
        bool global = boost::starts_with(name, "::");
        auto ns = name.substr(global ? 2 : 0, global ? (pos > 2 ? pos - 2 : 0) : pos);
        auto var = name.substr(pos + 2);

        // An empty namespace is the top scope
        if (ns.empty()) {
            auto variable = top_scope()->get(var);
            return variable ? variable->value() : nullptr;
        }

        // Lookup the namespace
        auto scope = find_scope(ns);
        if (scope) {
            auto variable = scope->get(var);
            return variable ? variable->value() : nullptr;
        }

        // Warn if the scope was not found
        if (_catalog && evaluator && position) {
            // TODO: find the class on the node

            string message;
            if (!_catalog->find_class(types::klass(ns))) {
                message = (boost::format("could not look up variable $%1% because class '%2%' is not defined.") % name % ns).str();
            } else if (!_catalog->find_resource(types::resource("class", ns))) {
                message = (boost::format("could not look up variable $%1% because class '%2%' has not been declared.") % name % ns).str();
            }
            if (!message.empty()) {
                evaluator->warn(*position, message);
            }
        }
        return nullptr;
    }

    shared_ptr<value const> context::lookup(size_t index) const
    {
        // Walk the match scope stack for a non-null set of matches
        for (auto it = _match_stack.rbegin(); it != _match_stack.rend(); ++it) {
            auto const& matches = *it;
            if (matches) {
                if (index >= matches->size()) {
                    return nullptr;
                }
                return (*matches)[index];
            }
        }
        return nullptr;
    }

    match_scope context::create_match_scope()
    {
        return match_scope { *this };
    }

    local_scope context::create_local_scope(shared_ptr<runtime::scope> scope)
    {
        return local_scope { *this, rvalue_cast(scope) };
    }

}}  // namespace puppet::runtime
