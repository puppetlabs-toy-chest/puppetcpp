#include <puppet/runtime/values/value.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    ostream& operator<<(ostream& os, defaulted const&)
    {
        os << "default";
        return os;
    }

    bool operator==(defaulted const&, defaulted const&)
    {
        return true;
    }

    bool operator!=(defaulted const& left, defaulted const& right)
    {
        return !(left == right);
    }

    size_t hash_value(defaulted const&)
    {
        return 0;
    }

}}}  // namespace puppet::runtime::values
