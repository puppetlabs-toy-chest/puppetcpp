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

    values::type alias::generalize() const
    {
        // Aliases don't generalize the resolved type; they generalize to themselves
        return *this;
    }

    bool alias::is_instance(values::value const& value, recursion_guard& guard) const
    {
        auto result = guard.add(*this, &value);
        if (result.recursed()) {
            return result.value();
        }
        result.value(_resolved_type->is_instance(value, guard));
        return result.value();
    }

    bool alias::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        if (auto alias = boost::get<types::alias>(&other.get())) {
            auto target = &alias->_resolved_type->dereference();

            // Aliases are assignable if they ultimately point at the same resolved type
            if (&_resolved_type->dereference() == target) {
                return true;
            }
            // Check if this alias refers to a variant that may contain an alias that resolves to the other alias
            if (auto variant = boost::get<types::variant>(_resolved_type.get())) {
                for (auto& t : variant->types()) {
                    if (&t->dereference() == target) {
                        return true;
                    }
                }
            }
        }

        // Recurse on the resolved type
        auto result = guard.add(*this, &other);
        if (result.recursed()) {
            return result.value();
        }
        result.value(_resolved_type->is_assignable(other, guard));
        return result.value();
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

    recursion_guard::key::key(types::alias const& alias, void const* other) :
        _resolved(alias.resolved_type()),
        _other(other)
    {
    }

    values::type const& recursion_guard::key::resolved() const
    {
        return _resolved;
    }

    void const* recursion_guard::key::other() const
    {
        return _other;
    }

    bool recursion_guard::result::recursed() const
    {
        return _recursed;
    }

    bool recursion_guard::result::value() const
    {
        return _iterator->second;
    }

    void recursion_guard::result::value(bool val)
    {
        _iterator->second = val;
    }

    recursion_guard::result::result(recursion_guard::map_type::iterator iterator, bool recursed) :
        _iterator(rvalue_cast(iterator)),
        _recursed(recursed)
    {
    }

    recursion_guard::result recursion_guard::add(types::alias const& alias, void const* other)
    {
        auto result = _map.emplace(key{ alias, other }, false);
        return { rvalue_cast(result.first), !result.second };
    }

    bool operator==(recursion_guard::key const& left, recursion_guard::key const& right)
    {
        return &left.resolved() == &right.resolved() && left.other() == right.other();
    }

    size_t hash_value(recursion_guard::key const& key)
    {
        size_t seed = 0;
        // Just use the addresses
        boost::hash_combine(seed, &key.resolved());
        if (key.other()) {
            boost::hash_combine(seed, key.other());
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
