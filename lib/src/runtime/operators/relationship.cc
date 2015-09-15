#include <puppet/runtime/operators/relationship.hpp>
#include <puppet/runtime/expression_evaluator.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    static value add_relationship(binary_context& context, runtime::relationship relationship)
    {
        auto& evaluator = context.evaluator();
        auto catalog = evaluator.evaluation_context().catalog();
        if (!catalog) {
            throw evaluator.create_exception(context.left_position(), "relationship expressions are not supported.");
        }

        // Populate an array of resource references from the right-hand side
        values::array result;
        each_resource(context.right(), [&](types::resource const& target) {
            result.push_back(target);
        }, [&](string const& message) {
            throw evaluator.create_exception(context.right_position(), message);
        });

        // Add the relationship; it will be evaluated upon finalization
        catalog->add_relationship(
           resource_relationship(
               evaluator.compilation_context(),
               rvalue_cast(context.left()),
               context.left_position(),
               rvalue_cast(context.right()),
               context.right_position(),
               relationship
           ));

        return result;
    }

    value in_edge::operator()(binary_context& context) const
    {
        return add_relationship(context, relationship::before);
    }

    value in_edge_subscribe::operator()(binary_context& context) const
    {
        return add_relationship(context, relationship::notify);
    }

    value out_edge::operator()(binary_context& context) const
    {
        return add_relationship(context, relationship::require);
    }

    value out_edge_subscribe::operator()(binary_context& context) const
    {
        return add_relationship(context, relationship::subscribe);
    }

}}}  // namespace puppet::runtime::operators
