#include <puppet/runtime/types/numeric.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* numeric::name()
    {
        return "Numeric";
    }

    ostream& operator<<(ostream& os, numeric const&)
    {
        os << numeric::name();
        return os;
    }

    bool operator==(numeric const&, numeric const&)
    {
        return true;
    }

}}}  // namespace puppet::runtime::types
