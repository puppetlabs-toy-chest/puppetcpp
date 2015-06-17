#include <puppet/runtime/values/value.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    // This exists simply to forward the call to the implementation in the values namespace
    // This function is legally allowed to be locally forward declared in a type's is_instance implementation.
    bool unsafe_is_instance(void const* value, void const* type)
    {
        return values::is_instance(*reinterpret_cast<values::value const*>(value), *reinterpret_cast<values::type const*>(type));
    }

}}}  // namespace puppet::runtime::types
