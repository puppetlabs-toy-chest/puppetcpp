#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    hash const hash::instance;

    hash::hash(
        unique_ptr<values::type> key_type,
        unique_ptr<values::type> value_type,
        int64_t from,
        int64_t to) :
            _key_type(rvalue_cast(key_type)),
            _value_type(rvalue_cast(value_type)),
            _from(from),
            _to(to)
    {
        if (!_key_type) {
            _key_type.reset(new values::type(scalar()));
        }
        if (!_value_type) {
            _value_type.reset(new values::type(data()));
        }
    }

    hash::hash(hash const& other) :
        _key_type(new values::type(other.key_type())),
        _value_type(new values::type(other.value_type())),
        _from(other.from()),
        _to(other.to())
    {
    }

    hash& hash::operator=(hash const& other)
    {
        _key_type.reset(new values::type(other.key_type()));
        _value_type.reset(new values::type(other.value_type()));
        _from = other.from();
        _to = other.to();
        return *this;
    }

    values::type const& hash::key_type() const
    {
        return *_key_type;
    }

    values::type const& hash::value_type() const
    {
        return *_value_type;
    }

    int64_t hash::from() const
    {
        return _from;
    }

    int64_t hash::to() const
    {
        return _to;
    }

    char const* hash::name()
    {
        return "Hash";
    }

    values::type hash::generalize() const
    {
        return types::hash{
            make_unique<values::type>(_key_type->generalize()),
            make_unique<values::type>(_value_type->generalize())
        };
    }

    bool hash::is_instance(values::value const& value, recursion_guard& guard) const
    {
        // Check for hash
        auto ptr = value.as<values::hash>();
        if (!ptr) {
            return false;
        }

        // Check for size is range
        int64_t size = static_cast<int64_t>(ptr->size());
        if (!(_to < _from ? (size >= _to && size <= _from) : (size >= _from && size <= _to))) {
            return false;
        }

        // Check that each key and value is of the appropriate types
        for (auto const& kvp : *ptr) {
            if (!_key_type->is_instance(kvp.key(), guard) || !_value_type->is_instance(kvp.value(), guard)) {
                return false;
            }
        }
        return true;
    }

    bool hash::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        int64_t from, to;
        if (auto hash = boost::get<types::hash>(&other)) {
            // Ensure key and value types are assignable
            if (!_key_type->is_assignable(*hash->_key_type, guard) ||
                !_value_type->is_assignable(*hash->_value_type, guard)) {
                return false;
            }
            from = hash->_from;
            to = hash->_to;
        } else if (auto structure = boost::get<types::structure>(&other)) {
            // Ensure the struct's key is an instance of the key type (must be String) and the value types match
            // Ensure value types are assignable
            for (auto& kvp : structure->schema()) {
                if (!_key_type->is_instance(types::structure::to_key(*kvp.first), guard) ||
                    !_value_type->is_assignable(*kvp.second, guard)) {
                    return false;
                }
            }
            from = to = structure->schema().size();
        } else {
            // Not a Hash or Struct
            return false;
        }
        return std::min(from, to) >= std::min(_from, _to) &&
               std::max(from, to) <= std::max(_from, _to);
    }

    void hash::write(ostream& stream, bool expand) const
    {
        stream << hash::name();

        // Write empty hashes in a special format, without the key and value types
        if (_from == 0 && _to == 0) {
            stream << "[0, 0]";
            return;
        }

        bool from_default = _from == 0;
        bool to_default = _to == numeric_limits<int64_t>::max();

        if (from_default && to_default) {
            if (*_key_type != scalar::instance || *_value_type != data::instance) {
                stream << '[';
                _key_type->write(stream, false);
                stream << ", ";
                _value_type->write(stream, false);
                stream << ']';
            }
            return;
        }

        stream << '[';
        _key_type->write(stream, false);
        stream << ", ";
        _value_type->write(stream, false);
        stream << ", ";
        if (from_default) {
            stream << "default";
        } else {
            stream << _from;
        }
        stream << ", ";
        if (to_default) {
            stream << "default";
        } else {
            stream << _to;
        }
        stream << ']';
    }

    ostream& operator<<(ostream& os, hash const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(hash const& left, hash const& right)
    {
        return left.from() == right.from() &&
               left.to() == right.to() &&
               left.key_type() == right.key_type() &&
               left.value_type() == right.value_type();
    }

    bool operator!=(hash const& left, hash const& right)
    {
        return !(left == right);
    }

    size_t hash_value(hash const& type)
    {
        static const size_t name_hash = boost::hash_value(hash::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.key_type());
        boost::hash_combine(seed, type.value_type());
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
