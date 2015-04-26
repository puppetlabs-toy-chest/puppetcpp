#include <puppet/runtime/values/value.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    // This exists simply to forward the call to the implementation in the values namespace
    // This function is legally allowed to be locally forward declared in a type's is_instance implementation.
    bool is_instance(values::value const& value, values::type const& type)
    {
        return values::is_instance(value, type);
    }

}}}  // namespace puppet::runtime::types
