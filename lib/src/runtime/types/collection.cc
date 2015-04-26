#include <puppet/runtime/types/collection.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    collection::collection(int64_t from, int64_t to) :
        _from(from),
        _to(to)
    {
    }

    int64_t collection::from() const
    {
        return _from;
    }

    int64_t collection::to() const
    {
        return _to;
    }

    const char* collection::name()
    {
        return "Collection";
    }

    ostream& operator<<(ostream& os, collection const& type)
    {
        os << collection::name();
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

    bool operator==(collection const& left, collection const& right)
    {
        return left.from() == right.from() && left.to() == right.to();
    }

}}}  // namespace puppet::runtime::types
