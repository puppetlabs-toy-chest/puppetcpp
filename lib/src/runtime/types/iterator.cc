#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    iterator::iterator(unique_ptr<values::type> type) :
        _type(rvalue_cast(type))
    {
    }

    iterator::iterator(iterator const& other) :
        _type(other.type() ? new values::type(*other.type()) : nullptr)
    {
    }

    iterator& iterator::operator=(iterator const& other)
    {
        _type.reset(other.type() ? new values::type(*other.type()) : nullptr);
        return *this;
    }

    unique_ptr<values::type> const& iterator::type() const
    {
        return _type;
    }

    char const* iterator::name()
    {
        return "Iterator";
    }

    bool iterator::is_instance(values::value const& value) const
    {
        auto iterator = value.as<values::iterator>();
        if (!iterator) {
            return false;
        }

        bool match = true;
        if (_type) {
            auto tuple = boost::get<types::tuple>(_type.get());
            iterator->each([&](auto const* key, auto const& value) {
                if (key) {
                    if (!tuple) {
                        match = false;
                        return false;
                    }
                    auto& types = tuple->types();
                    if (types.size() != 2) {
                        match = false;
                        return false;
                    }
                    match = types[0]->is_instance(*key) && types[1]->is_instance(value);
                } else {
                    match = _type->is_instance(value);
                }
                return match;
            });
        }
        return match;
    }

    bool iterator::is_specialization(values::type const& other) const
    {
        // If this iterator has a specialization, the other iterator cannot be a specialization
        if (_type) {
            return false;
        }
        // Check that the other iterator is specialized
        auto iterator = boost::get<types::iterator>(&other);
        return iterator && iterator->type();
    }

    ostream& operator<<(ostream& os, types::iterator const& type)
    {
        os << types::iterator::name();
        if (!type.type()) {
            return os;
        }
        os << '[' << *type.type() << ']';
        return os;
    }

    bool operator==(iterator const& left, iterator const& right)
    {
        if (!left.type() && !right.type()) {
            return true;
        } else if (left.type() || right.type()) {
            return false;
        }
        return *left.type() == *right.type();
    }

    bool operator!=(iterator const& left, iterator const& right)
    {
        return !(left == right);
    }

    size_t hash_value(types::iterator const& type)
    {
        static const size_t name_hash = boost::hash_value(types::iterator::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        if (type.type()) {
            boost::hash_combine(seed, *type.type());
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
