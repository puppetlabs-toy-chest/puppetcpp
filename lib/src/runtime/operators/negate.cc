#include <puppet/runtime/operators/negate.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <boost/format.hpp>
#include <limits>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    struct negate_visitor : boost::static_visitor<value>
    {
        negate_visitor(token_position const& position) :
            _position(position)
        {
        }

        result_type operator()(int64_t operand) const
        {
            if (operand == numeric_limits<int64_t>::min()) {
                throw evaluation_exception(_position, (boost::format("negation of %1% results in an arithmetic overflow.") % operand).str());
            }
            return -operand;
        }

        result_type operator()(long double operand) const
        {
            return -operand;
        }

        template <typename T>
        result_type operator()(T const& operand) const
        {
            throw evaluation_exception(_position, (boost::format("expected %1% for unary negation operator but found %2%.") % types::numeric::name() % get_type(operand)).str());
        }

     private:
        token_position const& _position;
    };

    value negate::operator()(unary_context& context) const
    {
        return boost::apply_visitor(negate_visitor(context.position()), dereference(context.operand()));
    }

}}}  // namespace puppet::runtime::operators
