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

    void collector::attributes(runtime::attributes attributes)
    {
        _attributes = rvalue_cast(attributes);
    }

    void collector::collect_resource(runtime::resource* resource, bool check)
    {
        if (!resource) {
            return;
        }

        // Realize the resource
        resource->realize();

        // Override any attributes
        resource->set(_attributes, true);

        // Check if the resource is already in the list
        if (check && find(_resources.begin(), _resources.end(), resource) != _resources.end()) {
            return;
        }

        // Add to the list
        _resources.push_back(resource);
    }

}}}  // namespace puppet::runtime::collectors
