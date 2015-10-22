#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    char const* defaulted::name()
    {
        return "Default";
    }

    bool defaulted::is_instance(values::value const& value) const
    {
        return value.is_default();
    }

    bool defaulted::is_specialization(values::type const& other) const
    {
        // No specializations of Default
        return false;
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

    bool operator!=(defaulted const& left, defaulted const& right)
    {
        return !(left == right);
    }

    size_t hash_value(defaulted const& type)
    {
        static const size_t name_hash = boost::hash_value(defaulted::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::types
