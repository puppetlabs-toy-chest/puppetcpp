#include <puppet/runtime/values/value.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    ostream& operator<<(ostream& os, defaulted const&)
    {
        os << "default";
        return os;
    }

    bool operator==(defaulted const&, defaulted const&)
    {
        return true;
    }

    bool operator!=(defaulted const& left, defaulted const& right)
    {
        return !(left == right);
    }

    size_t hash_value(defaulted const&)
    {
        static const size_t name_hash = boost::hash_value("default");

        std::size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::values
