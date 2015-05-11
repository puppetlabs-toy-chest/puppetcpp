#include <puppet/runtime/context.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    context::context(logging::logger& logger, function<void(lexer::token_position const&, string const&)> warn) :
        _logger(logger),
        _top(make_shared<scope>("Class[main]")),
        _warn(std::move(warn))
    {
    }

    void context::push()
    {
        auto parent = _stack.empty() ? _top : _stack.top();
        auto current = make_shared<scope>("", std::move(parent));
        _stack.push(current);
    }

    bool context::push(string name, string const& parent_name)
    {
        if (name.empty() || !_scopes.count(name)) {
            return false;
        }

        shared_ptr<scope> parent;
        if (!parent_name.empty()) {
            auto it = _scopes.find(parent_name);
            if (it == _scopes.end()) {
                // TODO: throw for not being able to find parent?
            } else {
                parent = it->second;
            }
        }
        if (!parent) {
            parent = _stack.empty() ? _top : _stack.top();
        }

        auto current = make_shared<scope>(name, std::move(parent));
        _stack.push(current);
        _scopes.emplace(make_pair(std::move(name), std::move(current)));
        return true;
    }

    bool context::pop()
    {
        if (_stack.empty()) {
            return false;
        }
        _stack.pop();
        return true;
    }

    value const* context::lookup(string const& name) const
    {
        auto pos = name.find_last_of("::");
        if (pos == string::npos) {
            // Not a qualified name
            return current().get(name);
        }
        if (pos == 0) {
            // Top scope lookup
            return _top->get(name);
        }

        // Name is qualified
        auto it = _scopes.find(name.substr(0, pos));
        if (it != _scopes.end()) {
            it->second->get(name.substr(pos + 2));
        } else {
            // TODO: emit a warning that the scope lookup failed
        }

        // Scope or variable not found
        return nullptr;
    }

    scope const& context::current() const
    {
        return _stack.empty() ? *_top : *_stack.top();
    }

    scope& context::current()
    {
        return _stack.empty() ? *_top : *_stack.top();
    }

    logging::logger const& context::logger() const
    {
        return _logger;
    }

    logging::logger& context::logger()
    {
        return _logger;
    }

    runtime::catalog const& context::catalog() const
    {
        return _catalog;
    }

    runtime::catalog& context::catalog()
    {
        return _catalog;
    }

    void context::warn(lexer::token_position const& position, string const& message) const
    {
        if (_warn) {
            _warn(position, message);
        }
    }

    ephemeral_scope::ephemeral_scope(context& ctx) :
        _ctx(ctx)
    {
        _ctx.push();
    }

    ephemeral_scope::~ephemeral_scope()
    {
        _ctx.pop();
    }

}}  // namespace puppet::runtime
