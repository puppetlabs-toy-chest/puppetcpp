#include <puppet/runtime/values/undef.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    ostream& operator<<(ostream& os, undef const&)
    {
        return os;
    }

}}}  // namespace puppet::runtime::values
