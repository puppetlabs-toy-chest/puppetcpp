#include <puppet/runtime/types/string.hpp>

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

    const char* string::name()
    {
        return "String";
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

}}}  // namespace puppet::runtime::types
