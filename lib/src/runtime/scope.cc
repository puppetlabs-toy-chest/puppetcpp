#include <puppet/runtime/scope.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    scope::scope(string name, string display_name, scope* parent) :
        _name(rvalue_cast(name)),
        _display_name(rvalue_cast(display_name)),
        _parent(parent)
    {
        // Emplace an empty set of matches to start
        _matches.emplace_front();
    }

    scope::scope(scope* parent)
    {
        // Emplace an empty set of matches to start
        _matches.emplace_front();
    }

    bool scope::ephemeral() const
    {
        return _name.empty() && _display_name.empty();
    }

    string const& scope::name() const
    {
        if (!_parent || !ephemeral()) {
            return _name;
        }
        return _parent->name();
    }

    string const& scope::display_name() const
    {
        if (!_parent || !ephemeral()) {
            return _display_name;
        }
        return _parent->display_name();
    }

    string scope::qualify(string const& name) const
    {
        auto& scope_name = this->name();
        if (scope_name.empty()) {
            return name;
        }
        return scope_name + "::" + name;
    }

    value const* scope::set(string name, value val, size_t line)
    {
        if (_variables.count(name)) {
            return nullptr;
        }
        auto result = _variables.emplace(make_pair(rvalue_cast(name), make_tuple(rvalue_cast(val), line)));
        return &std::get<0>(result.first->second);
    }

    value const* scope::get(string const& name) const
    {
        auto it = _variables.find(name);
        if (it != _variables.end()) {
            return &std::get<0>(it->second);
        }
        return _parent ? _parent->get(name) : nullptr;
    }

    size_t scope::where(string const& name)
    {
        auto it = _variables.find(name);
        if (it == _variables.end()) {
            return 0;
        }
        return std::get<1>(it->second);
    }

    scope const* scope::parent() const
    {
        return _parent;
    }

    void scope::set(smatch const& matches)
    {
        auto& current = _matches.front();
        current.clear();

        // Set the match variables
        for (size_t i = 0; i < matches.size(); ++i) {
            current.emplace_back(matches.str(i));
        }
    }

    value const* scope::get(size_t index) const
    {
        // Look for a non-empty set of matches
        // The first non-empty set wins
        for (auto const& matches : _matches) {
            if (matches.empty()) {
                continue;
            }
            if (index >= matches.size()) {
                return nullptr;
            }
            return &matches[index];
        }

        // Check the parent scope
        return _parent ? _parent->get(index) : nullptr;
    }

    void scope::push_matches()
    {
        _matches.emplace_front();
    }

    void scope::pop_matches()
    {
        // Pop all but the "top" set
        if (_matches.size() > 1) {
            _matches.pop_front();
        }
    }

    ostream& operator<<(ostream& os, scope const& s)
    {
        os << "Scope(" << s.display_name() << ")";
        return os;
    }

    match_variable_scope::match_variable_scope(scope& current) :
        _current(current)
    {
        _current.push_matches();
    }

    match_variable_scope::~match_variable_scope()
    {
        _current.pop_matches();
    }

}}  // namespace puppet::runtime
