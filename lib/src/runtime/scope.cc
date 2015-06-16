#include <puppet/runtime/scope.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    assigned_variable::assigned_variable(values::value value, shared_ptr<string> path, size_t line) :
        _value(rvalue_cast(value)),
        _path(rvalue_cast(path)),
        _line(line)
    {
        if (!_path) {
            throw runtime_error("expected path");
        }
    }

    values::value const& assigned_variable::value() const
    {
        return _value;
    }

    string const& assigned_variable::path() const
    {
        return *_path;
    }

    size_t assigned_variable::line() const
    {
        return _line;
    }

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
        if (boost::starts_with(name, "::")) {
            return name.substr(2);
        }

        auto& scope_name = this->name();
        if (scope_name.empty()) {
            return name;
        }
        return scope_name + "::" + name;
    }

    assigned_variable const* scope::set(string name, values::value value, shared_ptr<string> path, size_t line)
    {
        if (_variables.count(name)) {
            return nullptr;
        }
        auto result = _variables.emplace(make_pair(rvalue_cast(name), assigned_variable(rvalue_cast(value), rvalue_cast(path), line)));
        return &result.first->second;
    }

    assigned_variable const* scope::get(string const& name) const
    {
        auto it = _variables.find(name);
        if (it != _variables.end()) {
            return &it->second;
        }
        return _parent ? _parent->get(name) : nullptr;
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
