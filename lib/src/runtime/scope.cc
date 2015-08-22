#include <puppet/runtime/scope.hpp>
#include <puppet/runtime/catalog.hpp>
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

    scope::scope(shared_ptr<scope> parent, runtime::resource const* resource) :
        _parent(rvalue_cast(parent)),
        _resource(resource)
    {
        if (!_parent) {
            throw runtime_error("expected a parent scope.");
        }
    }

    scope::scope(shared_ptr<facts::provider> facts) :
        _facts(rvalue_cast(facts)),
        _resource(nullptr)
    {
    }

    shared_ptr<scope> const& scope::parent() const
    {
        return _parent;
    }

    runtime::resource const* scope::resource() const
    {
        if (_resource) {
            return _resource;
        }
        return _parent ? _parent->resource() : nullptr;
    }

    void scope::resource(runtime::resource const* resource)
    {
        _resource = resource;
    }

    string scope::qualify(string const& name) const
    {
        if (boost::starts_with(name, "::")) {
            return name.substr(2);
        }

        if (!_resource) {
            return name;
        }

        return _resource->type().title() + "::" + name;
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
        if (!s.resource()) {
            return os;
        }
        os << "Scope(" << s.resource()->type() << ")";
        return os;
    }

}}  // namespace puppet::runtime
