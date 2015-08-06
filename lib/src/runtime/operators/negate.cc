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
        negate_visitor(unary_context& context) :
            _context(context)
        {
        }

        result_type operator()(int64_t operand) const
        {
            if (operand == numeric_limits<int64_t>::min()) {
                throw _context.evaluator().create_exception(_context.position(), (boost::format("negation of %1% results in an arithmetic overflow.") % operand).str());
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
            throw _context.evaluator().create_exception(_context.position(), (boost::format("expected %1% for unary negation operator but found %2%.") % types::numeric::name() % get_type(operand)).str());
        }

     private:
        unary_context& _context;
    };

    value negate::operator()(unary_context& context) const
    {
        negate_visitor visitor(context);
        return boost::apply_visitor(visitor, dereference(context.operand()));
    }

}}}  // namespace puppet::runtime::operators
