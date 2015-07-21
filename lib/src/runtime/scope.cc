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

    scope::scope(shared_ptr<scope> parent, string name, string display_name) :
        _parent(rvalue_cast(parent)),
        _name(rvalue_cast(name)),
        _display_name(rvalue_cast(display_name))
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

    ostream& operator<<(ostream& os, scope const& s)
    {
        os << "Scope(" << s.display_name() << ")";
        return os;
    }

}}  // namespace puppet::runtime
