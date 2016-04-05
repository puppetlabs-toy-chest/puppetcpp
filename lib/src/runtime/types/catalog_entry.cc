#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    char const* catalog_entry::name()
    {
        return "CatalogEntry";
    }

    bool catalog_entry::is_instance(values::value const& value, recursion_guard& guard) const
    {
        return resource::instance.is_instance(value, guard) || klass::instance.is_instance(value, guard);
    }

    bool catalog_entry::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        if (boost::get<catalog_entry>(&other)) {
            return true;
        }
        return resource::instance.is_assignable(other, guard) || klass::instance.is_assignable(other, guard);
    }

    void catalog_entry::write(ostream& stream, bool expand) const
    {
        stream << catalog_entry::name();
    }

    ostream& operator<<(ostream& os, catalog_entry const& type)
    {
        type.write(os);
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
