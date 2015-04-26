#include <puppet/runtime/types/undef.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* undef::name()
    {
        return "Undef";
    }

    ostream& operator<<(ostream& os, undef const&)
    {
        os << undef::name();
        return os;
    }

    bool operator==(undef const&, undef const&)
    {
        return true;
    }

}}}  // namespace puppet::runtime::types
