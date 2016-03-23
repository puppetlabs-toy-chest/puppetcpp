#include <puppet/compiler/evaluation/collectors/list_collector.hpp>
#include <puppet/compiler/evaluation/context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace collectors {

    list_collector::list_collector(list_type list) :
        _list(rvalue_cast(list))
    {
        // Take a shared reference on the AST (assumes all entries in the list come from the same AST)
        if (!_list.empty()) {
            _tree = _list.front().second.tree->shared_from_this();
        }
    }

    void list_collector::detect_uncollected(evaluation::context& context) const
    {
        if (_list.empty()) {
            return;
        }
        // Throw an exception for the first uncollected resource
        auto& front = _list.front();
        throw evaluation_exception((boost::format("resource %1% does not exist in the catalog.") % front.first).str(), front.second, context.backtrace());
    }

    void list_collector::collect(evaluation::context& context)
    {
        auto& catalog = context.catalog();

        auto it = _list.begin();

        // Loop through any remaining resources in the list and realize them
        while (it != _list.end()) {
            auto resource = catalog.find(it->first);
            if (!resource) {
                ++it;
                continue;
            }

            // Collect the resource
            collect_resource(context, *resource);

            // Remove from the list
            _list.erase(it++);
        }
    }

}}}}  // namespace puppet::compiler::evaluation::collectors
