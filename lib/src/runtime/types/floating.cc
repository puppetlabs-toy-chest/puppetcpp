#include <puppet/runtime/types/floating.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    floating::floating(long double from, long double to) :
        _from(from),
        _to(to)
    {
    }

    long double floating::from() const
    {
        return _from;
    }

    long double floating::to() const
    {
        return _to;
    }

    const char* floating::name()
    {
        return "Float";
    }

    ostream& operator<<(ostream& os, floating const& type)
    {
        os << floating::name();
        // BUG: fix direct floating point comparison
        bool from_default = type.from() == numeric_limits<long double>::min();
        bool to_default = type.to() == numeric_limits<long double>::max();
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

    bool operator==(floating const& left, floating const& right)
    {
        return left.from() == right.from() && left.to() == right.to();
    }

}}}  // namespace puppet::runtime::types
