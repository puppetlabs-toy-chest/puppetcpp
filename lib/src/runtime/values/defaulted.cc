#include <puppet/runtime/values/defaulted.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    ostream& operator<<(ostream& os, defaulted const&)
    {
        os << "default";
        return os;
    }

}}}  // namespace puppet::runtime::values
