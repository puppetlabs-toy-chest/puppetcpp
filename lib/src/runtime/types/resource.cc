#include <puppet/runtime/types/resource.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* resource::name()
    {
        return "Resource";
    }

    ostream& operator<<(ostream& os, resource const&)
    {
        os << resource::name();
        return os;
    }

    bool operator==(resource const&, resource const&)
    {
        return true;
    }

}}}  // namespace puppet::runtime::types
