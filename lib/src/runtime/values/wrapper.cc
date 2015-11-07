#include <puppet/runtime/values/value.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    template <>
    value wrapper<value>::_undef{};

}}}  // namespace puppet::runtime::values
