#include <puppet/runtime/collectors/list_collector.hpp>
#include <puppet/runtime/catalog.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace collectors {

    list_collector::list_collector(shared_ptr<compiler::context> context, list<pair<types::resource, lexer::position>> list) :
        _context(rvalue_cast(context)),
        _list(rvalue_cast(list))
    {
    }

    void list_collector::detect_uncollected() const
    {
        if (_list.empty()) {
            return;
        }
        auto& front = _list.front();
        throw evaluation_exception(
            (boost::format("resource %1% does not exist in the catalog.") % front.first).str(),
            _context,
            front.second);
    }

    void list_collector::collect(runtime::context& context)
    {
        auto catalog = context.catalog();
        if (!catalog) {
            return;
        }

        auto it = _list.begin();

        // Loop through any remaining resources in the list and realize them
        while (it != _list.end()) {
            auto resource = catalog->find_resource(it->first);
            if (!resource) {
                ++it;
                continue;
            }

            // Collect the resource
            collect_resource(*catalog, *resource);

            // Remove from the list
            _list.erase(it++);
        }
    }

}}}  // namespace puppet::runtime::collectors
