#include <puppet/runtime/values/value.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    ostream& operator<<(ostream& os, undef const&)
    {
        return os;
    }

    bool operator==(undef const&, undef const&)
    {
        return true;
    }

    bool operator!=(undef const& left, undef const& right)
    {
        return !(left == right);
    }

    size_t hash_value(values::undef const&)
    {
        static const size_t name_hash = boost::hash_value("undef");

        std::size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::values
