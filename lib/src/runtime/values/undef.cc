#include <puppet/runtime/values/value.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    ostream& operator<<(ostream& os, undef const&)
    {
        return os;
    }

    bool operator==(undef const&, undef const&)
    {
        return true;
    }

    bool operator!=(undef const& left, undef const& right)
    {
        return !(left == right);
    }

    size_t hash_value(values::undef const&)
    {
        return 0;
    }

}}}  // namespace puppet::runtime::values
