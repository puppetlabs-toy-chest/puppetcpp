#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    iterable::iterable(unique_ptr<values::type> type) :
        _type(rvalue_cast(type))
    {
    }

    iterable::iterable(iterable const& other) :
        _type(other.type() ? new values::type(*other.type()) : nullptr)
    {
    }

    iterable& iterable::operator=(iterable const& other)
    {
        _type.reset(other.type() ? new values::type(*other.type()) : nullptr);
        return *this;
    }

    unique_ptr<values::type> const& iterable::type() const
    {
        return _type;
    }

    char const* iterable::name()
    {
        return "Iterable";
    }

    bool iterable::is_instance(values::value const& value) const
    {
        // Check for array
        if (auto array = value.as<values::array>()) {
            if (_type) {
                for (auto& element : *array) {
                    if (!_type->is_instance(*element)) {
                        return false;
                    }
                }
            }
            return true;
        }

        // Check for hash
        if (auto hash = value.as<values::hash>()) {
            if (_type) {
                auto tuple = boost::get<types::tuple>(_type.get());
                if (!tuple) {
                    return false;
                }

                auto& types = tuple->types();
                if (types.size() != 2) {
                    return false;
                }

                // Check that all the keys and values are instances of the tuple
                for (auto& kvp : *hash) {
                    if (!types[0]->is_instance(kvp.key())) {
                        return false;
                    }
                    if (!types[1]->is_instance(kvp.value())) {
                        return false;
                    }
                }
            }
            return true;
        }

        // Check for an integral value
        if (auto integer = value.as<int64_t>()) {
            if (_type) {
                // Check for Integer[0, N-1]
                auto type = boost::get<types::integer>(_type.get());
                if (!type || type->from() != 0 || type->to() != (*integer - 1)) {
                    return false;
                }
            }
            return *integer >= 0;
        }

        // Check for type
        if (auto type = value.as<values::type>()) {
            if (_type && *_type != *type) {
                return false;
            }

            // Check for Integer
            if (auto integer = boost::get<types::integer>(type)) {
                return integer->iterable();
            }

            // Check for Enum
            if (boost::get<types::enumeration>(type)) {
                return true;
            }
            return false;
        }

        // TODO: add support for Iterator
        return false;
    }

    bool iterable::is_specialization(values::type const& other) const
    {
        // If this iterable has a specialization, the other iterable cannot be a specialization
        if (_type) {
            return false;
        }
        // Check that the other iterable is specialized
        auto iterable = boost::get<types::iterable>(&other);
        return iterable && iterable->type();
    }

    ostream& operator<<(ostream& os, types::iterable const& type)
    {
        os << types::iterable::name();
        if (!type.type()) {
            return os;
        }
        os << '[' << *type.type() << ']';
        return os;
    }

    bool operator==(iterable const& left, iterable const& right)
    {
        if (!left.type() && !right.type()) {
            return true;
        } else if (left.type() || right.type()) {
            return false;
        }
        return *left.type() == *right.type();
    }

    bool operator!=(iterable const& left, iterable const& right)
    {
        return !(left == right);
    }

    size_t hash_value(types::iterable const& type)
    {
        static const size_t name_hash = boost::hash_value(types::iterable::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        if (type.type()) {
            boost::hash_combine(seed, *type.type());
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
