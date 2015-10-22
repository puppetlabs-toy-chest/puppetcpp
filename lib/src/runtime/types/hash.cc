#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    hash::hash(
        unique_ptr<values::type> key_type,
        unique_ptr<values::type> element_type,
        int64_t from,
        int64_t to) :
            _key_type(rvalue_cast(key_type)),
            _element_type(rvalue_cast(element_type)),
            _from(from),
            _to(to)
    {
        if (!_key_type) {
            _key_type.reset(new values::type(scalar()));
        }
        if (!_element_type) {
            _element_type.reset(new values::type(data()));
        }
    }

    hash::hash(hash const& other) :
        _key_type(new values::type(other.key_type())),
        _element_type(new values::type(other.element_type())),
        _from(other.from()),
        _to(other.to())
    {
    }

    hash& hash::operator=(hash const& other)
    {
        _key_type.reset(new values::type(other.key_type()));
        _element_type.reset(new values::type(other.element_type()));
        _from = other.from();
        _to = other.to();
        return *this;
    }

    values::type const& hash::key_type() const
    {
        return *_key_type;
    }

    values::type const& hash::element_type() const
    {
        return *_element_type;
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

    bool hash::is_instance(values::value const& value) const
    {
        // Check for hash
        auto ptr = value.as<values::hash>();;
        if (!ptr) {
            return false;
        }

        // Check for size is range
        int64_t size = static_cast<int64_t>(ptr->size());
        if (!(_to < _from ? (size >= _to && size <= _from) : (size >= _from && size <= _to))) {
            return false;
        }

        // Check that each key and element is of the appropriate types
        for (auto const& kvp : *ptr) {
            if (_key_type->is_instance(*kvp.first)) {
                return false;
            }
            if (_element_type->is_instance(*kvp.second)) {
                return false;
            }
        }
        return true;
    }

    bool hash::is_specialization(values::type const& other) const
    {
        // For the other type to be a specialization, it must be an Hash or Struct
        // The key types must match
        // The element types must match
        // And the range of other needs to be inside of this type's range
        int64_t from, to;
        auto hash = boost::get<types::hash>(&other);
        if (hash) {
            // Check for key and element type match
            if (hash->key_type() != *_key_type || hash->element_type() != *_element_type) {
                return false;
            }
            from = hash->from();
            to = hash->to();
        } else {
            // Check for a Struct (the Hash must be Hash[String, T])
            auto structure = boost::get<types::structure>(&other);
            if (!structure || !boost::get<types::string>(_key_type.get())) {
                return false;
            }
            // Ensure all elements of the structure are of the element type
            for (auto& kvp : structure->schema()) {
                if (*kvp.second != *_element_type) {
                    return false;
                }
            }
            from = to = structure->schema().size();
        }
        // Check for equality
        if (from == _from && to == _to) {
            return false;
        }
        return std::min(from, to) >= std::min(_from, _to) &&
               std::max(from, to) <= std::max(_from, _to);
    }

    ostream& operator<<(ostream& os, hash const& type)
    {
        os << hash::name() << '[' << type.key_type() << ", " << type.element_type();
        bool from_default = type.from() == numeric_limits<int64_t>::min();
        bool to_default = type.to() == numeric_limits<int64_t>::max();
        if (from_default && to_default) {
            // Only output the types
            os << ']';
            return os;
        }
        os << ", ";
        if (from_default) {
            os << "default";
        } else {
            os << type.from();
        }
        os << ", ";
        if (to_default) {
            os << "default";
        } else {
            os << type.to();
        }
        os << ']';
        return os;
    }

    bool operator==(hash const& left, hash const& right)
    {
        return left.from() == right.from() &&
               left.to() == right.to() &&
               left.key_type() == right.key_type() &&
               left.element_type() == right.element_type();
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
        boost::hash_combine(seed, type.element_type());
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
