#include <puppet/runtime/types/runtime.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* runtime::name()
    {
        return "Runtime";
    }

    ostream& operator<<(ostream& os, types::runtime const& type)
    {
        os << runtime::name();
        if (type.runtime_name().empty()) {
            return os;
        }
        os << "[" << type.runtime_name();
        if (!type.type_name().empty()) {
            os << ", " << type.type_name();
        }
        os << "]";
        return os;
    }

    bool operator==(runtime const& left, runtime const& right)
    {
        return left.runtime_name() == right.runtime_name() && left.type_name() == right.type_name();
    }

}}}  // namespace puppet::runtime::types
