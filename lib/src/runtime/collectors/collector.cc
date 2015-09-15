#include <puppet/runtime/collectors/collector.hpp>
#include <puppet/runtime/catalog.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace collectors {

    void collector::detect_uncollected() const
    {
    }

    vector<resource*> const& collector::resources() const
    {
        return _resources;
    }

}}}  // namespace puppet::runtime::collectors
