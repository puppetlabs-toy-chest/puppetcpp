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

    bool iterable::is_instance(values::value const& value, recursion_guard& guard) const
    {
        // Check for string
        if (value.as<std::string>()) {
            return true;
        }

        // Check for array
        if (auto array = value.as<values::array>()) {
            if (_type) {
                for (auto& element : *array) {
                    if (!_type->is_instance(*element, guard)) {
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
                    if (!types[0]->is_instance(kvp.key(), guard) || !types[1]->is_instance(kvp.value(), guard)) {
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

        // Check for iterator
        if (auto iterator = value.as<values::iterator>()) {
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
                        match = types[0]->is_instance(*key, guard) && types[1]->is_instance(value, guard);
                    } else {
                        match = _type->is_instance(value, guard);
                    }
                    return match;
                });
            }
            return match;
        }
        return false;
    }

    bool iterable::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        // Check for Iterable
        if (auto iterable = boost::get<types::iterable>(&other)) {
            if (!_type) {
                return true;
            }
            if (!iterable->type()) {
                return false;
            }
            return _type->is_assignable(*iterable->type(), guard);
        }
        // Check for string
        if (boost::get<types::string>(&other)) {
            return !_type || _type->is_assignable(other, guard);
        }
        // Check for array
        if (auto array = boost::get<types::array>(&other)) {
            return !_type || _type->is_assignable(array->element_type(), guard);
        }
        // Check for hash
        if (auto hash = boost::get<types::hash>(&other)) {
            if (!_type) {
                return true;
            }
            auto tuple = boost::get<types::tuple>(_type.get());
            if (!tuple) {
                return false;
            }
            auto& types = tuple->types();
            if (types.size() != 2) {
                return false;
            }
            return types[0]->is_assignable(hash->key_type(), guard) && types[1]->is_assignable(hash->value_type(), guard);
        }
        // Check for Integer
        if (auto integer = boost::get<types::integer>(&other)) {
            if (!integer->iterable()) {
                return false;
            }
            return !_type || _type->is_assignable(other, guard);
        }
        // Check for Enum
        if (boost::get<types::enumeration>(&other)) {
            return !_type || _type->is_assignable(other, guard);
        }
        // Check for Iterator
        if (auto iterator = boost::get<types::iterator>(&other)) {
            if (!_type) {
                return true;
            }
            if (!iterator->type()) {
                return false;
            }
            return _type->is_assignable(*iterator->type(), guard);
        }
        return false;
    }

    void iterable::write(ostream& stream, bool expand) const
    {
        stream << types::iterable::name();
        if (!_type) {
            return;
        }
        stream << '[';
        _type->write(stream, false);
        stream << ']';
    }

    ostream& operator<<(ostream& os, types::iterable const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(iterable const& left, iterable const& right)
    {
        if (left.type() || right.type()) {
            if (!left.type() || !right.type()) {
                return false;
            }
            return *left.type() == *right.type();
        }
        return true;
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
