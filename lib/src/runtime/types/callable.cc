#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    char const* callable::name()
    {
        return "Callable";
    }

    bool callable::is_instance(values::value const& value) const
    {
        // TODO: implement
        return false;
    }

    bool callable::is_specialization(values::type const& other) const
    {
        // No specializations for Callable
        return false;
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

    bool operator!=(callable const& left, callable const& right)
    {
        return !(left == right);
    }

    size_t hash_value(callable const& type)
    {
        static const size_t name_hash = boost::hash_value(callable::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::types
