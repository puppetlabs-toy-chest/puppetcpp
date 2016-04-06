#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    string::string(int64_t from, int64_t to) :
        _from(from),
        _to(to)
    {
    }

    string::string(integer const& range) :
        _from(range.from()),
        _to(range.to())
    {
    }

    int64_t string::from() const
    {
        return _from;
    }

    int64_t string::to() const
    {
        return _to;
    }

    char const* string::name()
    {
        return "String";
    }

    bool string::is_instance(values::value const& value) const
    {
        auto ptr = value.as<std::string>();
        if (!ptr) {
            return false;
        }
        auto size = static_cast<int64_t>(ptr->size());
        return _to < _from ? (size >= _to && size <= _from) : (size >= _from && size <= _to);
    }

    bool string::is_specialization(values::type const& other) const
    {
        // Check for an String with a range inside of this type's range
        auto ptr = boost::get<types::string>(&other);
        if (!ptr) {
            return false;
        }
        // Check for equality
        if (ptr->from() == _from && ptr->to() == _to) {
            return false;
        }
        return std::min(ptr->from(), ptr->to()) >= std::min(_from, _to) &&
               std::max(ptr->from(), ptr->to()) <= std::max(_from, _to);
    }

    bool string::is_real(unordered_map<values::type const*, bool>& map) const
    {
        // String is a real type
        return true;
    }

    void string::write(ostream& stream, bool expand) const
    {
        stream << string::name();
        bool from_default = _from == numeric_limits<int64_t>::min();
        bool to_default = _to == numeric_limits<int64_t>::max();
        if (from_default && to_default) {
            // Only output the type name
            return;
        }
        stream << '[';
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

    ostream& operator<<(ostream& os, string const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(string const& left, string const& right)
    {
        return left.from() == right.from() && left.to() == right.to();
    }

    bool operator!=(string const& left, string const& right)
    {
        return !(left == right);
    }

    size_t hash_value(string const& type)
    {
        static const size_t name_hash = boost::hash_value(string::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
