#include <puppet/runtime/operators/relationship.hpp>
#include <puppet/runtime/expression_evaluator.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    static value add_relationship(binary_context& context, string const& attribute_name)
    {
        auto& evaluator = context.evaluator();
        auto catalog = evaluator.evaluation_context().catalog();
        if (!catalog) {
            throw evaluator.create_exception(context.left_position(), "relationship expressions are not supported.");
        }

        // Populate an array of resource references from the right-hand side
        values::array result;
        each_resource(context.right(), [&](types::resource const& target_resource) {
            // Locate the target in the catalog
            auto target = catalog->find_resource(target_resource);
            if (!target) {
                throw evaluator.create_exception(context.right_position(), (boost::format("cannot create relationship: resource %1% does not exist in the catalog.") % target_resource).str());
            }

            result.emplace_back(target_resource);
        }, [&](string const& message) {
            throw evaluator.create_exception(context.right_position(), message);
        });

        // Now associate each left-hand side resource with those on the right
        each_resource(context.left(),[&](types::resource const& source_resource) {
            // Locate the source in the catalog
            auto source = catalog->find_resource(source_resource);
            if (!source) {
                throw evaluator.create_exception(context.left_position(), (boost::format("cannot create relationship: resource %1% does not exist in the catalog.") % source_resource).str());
            }

            source->attributes().append(attribute_name, result, true /* remove duplicates */);
        }, [&](string const& message) {
            throw evaluator.create_exception(context.left_position(), message);
        });
        return result;
    }

    value in_edge::operator()(binary_context& context) const
    {
        return add_relationship(context, "before");
    }

    value in_edge_subscribe::operator()(binary_context& context) const
    {
        return add_relationship(context, "notify");
    }

    value out_edge::operator()(binary_context& context) const
    {
        return add_relationship(context, "require");
    }

    value out_edge_subscribe::operator()(binary_context& context) const
    {
        return add_relationship(context, "subscribe");
    }

}}}  // namespace puppet::runtime::operators
