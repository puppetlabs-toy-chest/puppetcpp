#include <puppet/runtime/types/defaulted.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* defaulted::name()
    {
        return "Default";
    }

    ostream& operator<<(ostream& os, defaulted const&)
    {
        os << defaulted::name();
        return os;
    }

    bool operator==(defaulted const&, defaulted const&)
    {
        return true;
    }

}}}  // namespace puppet::runtime::types
