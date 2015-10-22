#include <puppet/compiler/evaluation/operators/relationship.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/catalog.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    static value add_relationship(binary_operator_context const& context, compiler::relationship relationship)
    {
        // Populate an array of resource references from the right-hand side
        values::array result;
        context.right().each_resource([&](types::resource const& target) {
            result.push_back(target);
        }, [&](string const& message) {
            throw evaluation_exception(message, context.right_context());
        });

        // Add the relationship; it will be evaluated upon context finalization
        context.context().add(
            resource_relationship(
                relationship,
                rvalue_cast(context.left()),
                context.left_context(),
                rvalue_cast(context.right()),
                context.right_context()
            )
        );
        return result;
    }

    value in_edge::operator()(binary_operator_context const& context) const
    {
        return add_relationship(context, relationship::before);
    }

    value in_edge_subscribe::operator()(binary_operator_context const& context) const
    {
        return add_relationship(context, relationship::notify);
    }

    value out_edge::operator()(binary_operator_context const& context) const
    {
        return add_relationship(context, relationship::require);
    }

    value out_edge_subscribe::operator()(binary_operator_context const& context) const
    {
        return add_relationship(context, relationship::subscribe);
    }

}}}}  // namespace puppet::compiler::evaluation::operators
