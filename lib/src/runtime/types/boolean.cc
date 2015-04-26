#include <puppet/runtime/types/boolean.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* boolean::name()
    {
        return "Boolean";
    }

    ostream& operator<<(ostream& os, boolean const&)
    {
        os << boolean::name();
        return os;
    }

    bool operator==(boolean const&, boolean const&)
    {
        return true;
    }

}}}  // namespace puppet::runtime::types
