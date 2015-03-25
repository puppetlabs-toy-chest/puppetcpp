#include <puppet/runtime/scope.hpp>

using namespace std;

namespace puppet { namespace runtime {

    scope::scope(string name, shared_ptr<scope> parent) :
        _name(std::move(name)),
        _parent(std::move(parent))
    {
        // Emplace an empty set of matches to start
        _matches.emplace_front();
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
        return _parent ? _parent->get(name) : nullptr;
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
        os << "Scope(" << s.name() << ")";
        return os;
    }

}}  // namespace puppet::runtime