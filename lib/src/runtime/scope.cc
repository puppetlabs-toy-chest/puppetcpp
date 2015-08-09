#include <puppet/runtime/scope.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    assigned_variable::assigned_variable(shared_ptr<values::value const> value, shared_ptr<string> path, size_t line) :
        _value(rvalue_cast(value)),
        _path(rvalue_cast(path)),
        _line(line)
    {
    }

    shared_ptr<value const> const& assigned_variable::value() const
    {
        return _value;
    }

    string const* assigned_variable::path() const
    {
        return _path.get();
    }

    size_t assigned_variable::line() const
    {
        return _line;
    }

    scope::scope(shared_ptr<scope> parent, string name, string display_name) :
        _parent(rvalue_cast(parent)),
        _name(rvalue_cast(name)),
        _display_name(rvalue_cast(display_name))
    {
        if (!_parent) {
            throw runtime_error("expected a parent scope.");
        }
    }

    scope::scope(shared_ptr<facts::provider> facts) :
        _facts(rvalue_cast(facts)),
        _name(""),
        _display_name("Class[main]")
    {
    }

    string const& scope::name() const
    {
        if (!_name.empty()) {
            return _name;
        }
        return _parent ? _parent->name() : _name;
    }

    string const& scope::display_name() const
    {
        if (!_display_name.empty()) {
            return _display_name;
        }
        return _parent ? _parent->display_name() : _display_name;
    }

    shared_ptr<scope> const& scope::parent() const
    {
        return _parent;
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

    assigned_variable const* scope::set(string name, shared_ptr<values::value const> value, shared_ptr<string> path, size_t line)
    {
        // Check to see if the variable already exists
        auto it = _variables.find(name);
        if (it != _variables.end()) {
            return &it->second;
        }

        // If there's a fact provider, try get a fact of the given name before setting
        if (_facts) {
            auto previous = get(name);
            if (previous) {
                return previous;
            }
        }
        _variables.emplace(make_pair(rvalue_cast(name), assigned_variable(rvalue_cast(value), rvalue_cast(path), line)));
        return nullptr;
    }

    assigned_variable const* scope::get(string const& name)
    {
        auto it = _variables.find(name);
        if (it != _variables.end()) {
            return &it->second;
        }

        // Go up the parent if there is one
        if (_parent) {
            return _parent->get(name);
        }

        // Lookup the fact if there's a fact provider
        if (!_facts) {
            return nullptr;
        }
        auto value = _facts->lookup(name);
        if (!value) {
            return nullptr;
        }

        // Add the fact as an assigned variable
        return &_variables.emplace(make_pair(name, assigned_variable(rvalue_cast(value)))).first->second;
    }

    ostream& operator<<(ostream& os, scope const& s)
    {
        os << "Scope(" << s.display_name() << ")";
        return os;
    }

}}  // namespace puppet::runtime
