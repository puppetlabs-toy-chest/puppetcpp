#include <puppet/runtime/scope.hpp>

using namespace std;

namespace puppet { namespace runtime {

    scope::scope(string name, shared_ptr<scope> parent) :
        _name(std::move(name)),
        _parent(std::move(parent))
    {
    }

    string const& scope::name() const
    {
        return _name;
    }

    value const* scope::set(string name, value val, uint32_t line)
    {
        if (_variables.count(name)) {
            return nullptr;
        }
        auto entry = make_tuple(std::move(val), std::move(line));
        auto result = _variables.emplace(make_pair(std::move(name), std::move(entry)));
        return &std::get<0>(result.first->second);
    }

    value const* scope::get(string const& name) const
    {
        auto it = _variables.find(name);
        if (it != _variables.end()) {
            return &std::get<0>(it->second);
        }
        auto match = _matches.find(name);
        if (match != _matches.end()) {
            return &match->second;
        }
        return nullptr;
    }

    boost::optional<uint32_t> scope::where(string const& name) const
    {
        auto it = _variables.find(name);
        if (it != _variables.end()) {
            return std::get<1>(it->second);
        }
        return nullptr;
    }

    scope const* scope::parent() const
    {
        return _parent.get();
    }

    void scope::set(smatch const& matches)
    {
        _matches.clear();

        // Set the match variables
        for (size_t i = 0; i < matches.size(); ++i) {
            _matches[to_string(i)] = matches.str(i);
        }
    }

    ostream& operator<<(ostream& os, scope const& s)
    {
        os << "Scope(" << s.name() << ")";
        return os;
    }

}}  // namespace puppet::runtime