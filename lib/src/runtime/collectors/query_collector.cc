#include <puppet/runtime/collectors/query_collector.hpp>
#include <puppet/runtime/collectors/query_evaluator.hpp>
#include <puppet/runtime/catalog.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace collectors {

    query_collector::query_collector(shared_ptr<compiler::context> context, ast::collection_expression const& expression, shared_ptr<runtime::scope> scope) :
        _context(rvalue_cast(context)),
        _expression(expression),
        _scope(rvalue_cast(scope)),
        _index(0)
    {
        if (!_scope) {
            throw runtime_error("expected a scope.");
        }
    }

    void query_collector::collect(runtime::context& context)
    {
        auto catalog = context.catalog();
        if (!catalog) {
            return;
        }

        // TODO: support exported resources

        // Find the resources by type
        auto resources = catalog->find_resources(_expression.type().name());
        if (!resources) {
            return;
        }

        // Change to the current scope
        local_scope scope{ context, _scope };

        expression_evaluator evaluator(_context, context);
        query_evaluator query(evaluator, _expression.query());

        // Realize each resource that matches the query
        size_t size = resources->size();
        while (_index < size) {
            // Evaluate for this resource
            auto resource = (*resources)[_index++];
            if (!query.evaluate(*resource)) {
                continue;
            }

            // Collect the resource
            collect_resource(*catalog, *resource, false);
        }
    }

}}}  // namespace puppet::runtime::collectors
