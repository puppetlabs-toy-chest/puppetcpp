#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    integer const integer::instance;

    integer::integer(int64_t from, int64_t to) :
        _from(from),
        _to(to)
    {
        if (from > to) {
            throw runtime_error("from cannot be greater than to.");
        }
    }

    int64_t integer::from() const
    {
        return _from;
    }

    int64_t integer::to() const
    {
        return _to;
    }

    char const* integer::name()
    {
        return "Integer";
    }

    bool integer::iterable() const
    {
        return _from != numeric_limits<int64_t>::min() && _to != numeric_limits<int64_t>::max();
    }

    void integer::each(function<bool(int64_t, int64_t)> const& callback) const
    {
        if (!callback || !iterable()) {
            return;
        }

        // Check if we should go downwards
        bool backwards = _to < _from;
        for (int64_t index = 0, start = _from; (backwards && (start >= _to)) || (!backwards && (start <= _to)); ++index, start += (backwards ? -1 : 1)) {
            if (!callback(index, start)) {
                break;
            }
        }
    }

    values::type integer::generalize() const
    {
        return types::integer{};
    }

    bool integer::is_instance(values::value const& value, recursion_guard& guard) const
    {
        auto ptr = value.as<int64_t>();
        if (!ptr) {
            return false;
        }
        return _to < _from ? (*ptr >= _to && *ptr <= _from) : (*ptr >= _from && *ptr <= _to);
    }

    bool integer::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        // Check for an Integer with a range inside of this type's range
        auto ptr = boost::get<integer>(&other);
        if (!ptr) {
            return false;
        }
        return std::min(ptr->from(), ptr->to()) >= std::min(_from, _to) &&
               std::max(ptr->from(), ptr->to()) <= std::max(_from, _to);
    }

    void integer::write(ostream& stream, bool expand) const
    {
        stream << integer::name();
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

    ostream& operator<<(ostream& os, integer const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(integer const& left, integer const& right)
    {
        return left.from() == right.from() && left.to() == right.to();
    }

    bool operator!=(integer const& left, integer const& right)
    {
        return !(left == right);
    }

    size_t hash_value(integer const& type)
    {
        static const size_t name_hash = boost::hash_value(integer::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
