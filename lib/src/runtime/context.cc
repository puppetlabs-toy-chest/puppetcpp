#include <puppet/runtime/context.hpp>

using namespace std;

namespace puppet { namespace runtime {

    context::context() :
        _top(make_shared<scope>("Class[main]"))
    {
    }

    bool context::push(string name, string const& parent_name)
    {
        if (_scopes.count(name)) {
            return false;
        }

        auto parent = _top;
        if (!parent_name.empty()) {
            auto it = _scopes.find(parent_name);
            if (it != _scopes.end()) {
                parent = it->second;
            }
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

}}  // namespace puppet::runtime