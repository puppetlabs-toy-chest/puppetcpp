#include <puppet/compiler/evaluation/collectors/query_collector.hpp>
#include <puppet/compiler/evaluation/collectors/query_evaluator.hpp>
#include <puppet/compiler/evaluation/context.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace compiler { namespace evaluation { namespace collectors {

    query_collector::query_collector(ast::collector_expression const& expression, shared_ptr<evaluation::scope> scope) :
        _expression(expression),
        _scope(rvalue_cast(scope)),
        _index(0)
    {
        if (!_scope) {
            throw runtime_error("expected a scope.");
        }
    }

    void query_collector::collect(evaluation::context& context)
    {
        auto& catalog = context.catalog();

        // TODO: support exported resources

        // Change to the stored scope
        local_scope scope{ context, _scope };

        query_evaluator evaluator{ context, _expression.query };

        // Realize each resource that matches the query
        catalog.each(
            [&](compiler::resource& resource) {
                ++_index;

                // Evaluate the query this resource and collect if it matches
                if (evaluator.evaluate(resource)) {
                    collect_resource(context, resource, false);
                }
                return true;
            },
            _expression.type.name,
            _index);
    }

}}}}  // namespace puppet::compiler::evaluation::collectors
