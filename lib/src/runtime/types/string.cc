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

    ostream& operator<<(ostream& os, string const& type)
    {
        os << string::name();
        bool from_default = type.from() == numeric_limits<int64_t>::min();
        bool to_default = type.to() == numeric_limits<int64_t>::max();
        if (from_default && to_default) {
            // Only output the type name
            return os;
        }
        os << '[';
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
