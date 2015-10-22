#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    char const* catalog_entry::name()
    {
        return "CatalogEntry";
    }

    bool catalog_entry::is_instance(values::value const& value) const
    {
        return resource().is_instance(value) || klass().is_instance(value);
    }

    bool catalog_entry::is_specialization(values::type const& other) const
    {
        // Resource and Class types are specializations
        return boost::get<resource>(&other) ||
               boost::get<klass>(&other);
    }

    ostream& operator<<(ostream& os, catalog_entry const& type)
    {
        os << catalog_entry::name();
        return os;
    }

    bool operator==(catalog_entry const&, catalog_entry const&)
    {
        return true;
    }

    bool operator!=(catalog_entry const& left, catalog_entry const& right)
    {
        return !(left == right);
    }

    size_t hash_value(catalog_entry const& type)
    {
        static const size_t name_hash = boost::hash_value(catalog_entry::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::types
