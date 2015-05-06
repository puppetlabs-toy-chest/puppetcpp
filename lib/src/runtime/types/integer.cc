#include <puppet/runtime/types/integer.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    integer::integer(int64_t from, int64_t to) :
        _from(from),
        _to(to)
    {
    }

    int64_t integer::from() const
    {
        return _from;
    }

    int64_t integer::to() const
    {
        return _to;
    }

    const char* integer::name()
    {
        return "Integer";
    }

    bool integer::enumerable() const
    {
        return std::min(_from, _to) != numeric_limits<int64_t>::min() &&
               std::max(_from, _to) != numeric_limits<int64_t>::max();
    }

    void integer::each(function<bool(int64_t, int64_t)> const& callback) const
    {
        if (!callback || !enumerable()) {
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

    ostream& operator<<(ostream& os, integer const& type)
    {
        os << integer::name();
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

    bool operator==(integer const& left, integer const& right)
    {
        return left.from() == right.from() && left.to() == right.to();
    }

}}}  // namespace puppet::runtime::types
