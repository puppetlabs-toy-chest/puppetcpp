#include <puppet/runtime/types/klass.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* klass::name()
    {
        return "Class";
    }

    ostream& operator<<(ostream& os, klass const&)
    {
        os << klass::name();
        return os;
    }

    bool operator==(klass const&, klass const&)
    {
        return true;
    }

}}}  // namespace puppet::runtime::types
