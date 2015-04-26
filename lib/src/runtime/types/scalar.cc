#include <puppet/runtime/types/scalar.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* scalar::name()
    {
        return "Scalar";
    }

    ostream& operator<<(ostream& os, scalar const&)
    {
        os << scalar::name();
        return os;
    }

    bool operator==(scalar const&, scalar const&)
    {
        return true;
    }

}}}  // namespace puppet::runtime::types
