#include <puppet/runtime/types/runtime.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* runtime::name()
    {
        return "Runtime";
    }

    ostream& operator<<(ostream& os, runtime const&)
    {
        os << runtime::name();
        return os;
    }

    bool operator==(runtime const&, runtime const&)
    {
        return true;
    }

}}}  // namespace puppet::runtime::types
