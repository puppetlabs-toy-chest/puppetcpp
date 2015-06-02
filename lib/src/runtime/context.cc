#include <puppet/runtime/context.hpp>

using namespace std;

namespace puppet { namespace runtime {

    context::context(logging::logger& logger, compiler::node& node, runtime::catalog& catalog, function<void(lexer::position const&, string const&)> const& warning) :
        _logger(logger),
        _node(node),
        _catalog(catalog),
        _warn(warning)
    {
        // Add the top scope
        push_scope(*add_scope("", runtime::scope("Class[main]")));
    }

    logging::logger& context::logger()
    {
        return _logger;
    }

    logging::logger const& context::logger() const
    {
        return _logger;
    }

    compiler::node& context::node()
    {
        return _node;
    }

    compiler::node const& context::node() const
    {
        return _node;
    }

    runtime::catalog& context::catalog()
    {
        return _catalog;
    }

    runtime::catalog const& context::catalog() const
    {
        return _catalog;
    }

    runtime::scope& context::scope()
    {
        return *_scope_stack.back();
    }

    runtime::scope const& context::scope() const
    {
        return *_scope_stack.back();
    }

    runtime::scope& context::top()
    {
        return *_scope_stack.front();
    }

    runtime::scope const& context::top() const
    {
        return *_scope_stack.front();
    }

    runtime::scope* context::add_scope(string name, runtime::scope scope)
    {
        return &_scopes.emplace(make_pair(rvalue_cast(name), rvalue_cast(scope))).first->second;
    }

    runtime::scope* context::find_scope(string const& name)
    {
        auto it = _scopes.find(name);
        if (it == _scopes.end()) {
            return nullptr;
        }
        return &it->second;
    }

    void context::push_scope(runtime::scope& current)
    {
        _scope_stack.push_back(&current);
    }

    bool context::pop_scope()
    {
        // Don't pop the top scope
        if (_scope_stack.size() == 1) {
            return false;
        }
        _scope_stack.pop_back();
        return true;
    }

    void context::warn(lexer::position const& position, string const& message) const
    {
        if (!_warn) {
            return;
        }
        _warn(position, message);
    }

    ephemeral_scope::ephemeral_scope(runtime::context& context) :
        _context(context),
        _scope(string(), &context.scope())
    {
        _context.push_scope(_scope);
    }

    ephemeral_scope::~ephemeral_scope()
    {
        _context.pop_scope();
    }

}}  // namespace puppet::runtime
