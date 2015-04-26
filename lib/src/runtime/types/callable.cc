#include <puppet/runtime/types/callable.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* callable::name()
    {
        return "Callable";
    }

    ostream& operator<<(ostream& os, callable const&)
    {
        os << callable::name();
        // TODO: implement
        return os;
    }

    bool operator==(callable const&, callable const&)
    {
        // TODO: implement
        return true;
    }

}}}  // namespace puppet::runtime::types
