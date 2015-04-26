#include <puppet/runtime/types/catalog_entry.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    const char* catalog_entry::name()
    {
        return "CatalogEntry";
    }

    ostream& operator<<(ostream& os, catalog_entry const&)
    {
        os << catalog_entry::name();
        return os;
    }

    bool operator==(catalog_entry const&, catalog_entry const&)
    {
        return true;
    }

}}}  // namespace puppet::runtime::types
