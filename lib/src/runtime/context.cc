#include <puppet/runtime/context.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace runtime {

    context::context(runtime::catalog& catalog) :
        _catalog(catalog)
    {
        // Add the top scope
        push_scope(add_scope("", "Class[main]"));
    }

    runtime::catalog& context::catalog()
    {
        return _catalog;
    }

    runtime::scope& context::scope()
    {
        return *_scope_stack.back();
    }

    runtime::scope& context::top_scope()
    {
        return *_scope_stack.front();
    }

    runtime::scope* context::node_scope()
    {
        return _node_scope.get();
    }

    runtime::scope& context::node_or_top()
    {
        auto node = node_scope();
        if (node) {
            return *node;
        }
        return top_scope();
    }

    runtime::scope& context::add_scope(string name, string display_name, runtime::scope* parent)
    {
        runtime::scope scope{name, rvalue_cast(display_name), parent};
        return _scopes.emplace(make_pair(rvalue_cast(name), rvalue_cast(scope))).first->second;
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

    local_scope::local_scope(runtime::context& context, runtime::scope* scope) :
        _context(context),
        _scope(&context.scope())
    {
        if (scope) {
            _context.push_scope(*scope);
        } else {
            _context.push_scope(_scope);
        }
    }

    local_scope::~local_scope()
    {
        _context.pop_scope();
    }

    node_scope::node_scope(runtime::context& context, string name) :
        _context(context)
    {
        _context._node_scope.reset(new runtime::scope("node", rvalue_cast(name), &_context.top_scope()));
    }

    node_scope::~node_scope()
    {
        _context._node_scope.reset();
    }

}}  // namespace puppet::runtime
