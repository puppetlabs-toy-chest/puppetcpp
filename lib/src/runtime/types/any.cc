#include <puppet/runtime/types/any.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* any::name()
    {
        return "Any";
    }

    ostream& operator<<(ostream& os, any const&)
    {
        os << any::name();
        return os;
    }

    bool operator==(any const&, any const&)
    {
        return true;
    }

}}}  // namespace puppet::runtime::types
