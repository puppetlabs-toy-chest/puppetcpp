#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    alias::alias(std::string name, shared_ptr<values::type> resolved_type) :
        _name(rvalue_cast(name)),
        _resolved_type(rvalue_cast(resolved_type))
    {
        if (!_resolved_type) {
            throw runtime_error("expected a resolved type for an alias.");
        }
    }

    std::string const& alias::name() const
    {
        return _name;
    }

    values::type const& alias::resolved_type() const
    {
        return *_resolved_type;
    }

    bool alias::is_instance(values::value const& value) const
    {
        return _resolved_type->is_instance(value);
    }

    bool alias::is_specialization(values::type const& other) const
    {
        // No specializations of type aliases
        return false;
    }

    bool alias::is_real(unordered_map<values::type const*, bool>& map) const
    {
        // If the alias was already encountered, return the result
        auto it = map.find(_resolved_type.get());
        if (it != map.end()) {
            return it->second;
        }

        // Pretend it is not real until we determine that the resolved type is real
        it = map.emplace(_resolved_type.get(), false).first;
        return (it->second = _resolved_type->is_real(map));
    }

    void alias::write(ostream& stream, bool expand) const
    {
        stream << _name;

        if (expand) {
            stream << " (alias for ";
            _resolved_type->write(stream, false);
            stream << ")";
        }
    }

    ostream& operator<<(ostream& os, alias const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(alias const& left, alias const& right)
    {
        return left.name() == right.name();
    }

    bool operator!=(alias const& left, alias const& right)
    {
        return !(left == right);
    }

    size_t hash_value(alias const& type)
    {
        static const size_t name_hash = boost::hash_value("alias");

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.name());
        return seed;
    }

}}}  // namespace puppet::runtime::types
