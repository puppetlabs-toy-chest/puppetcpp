#include <puppet/compiler/evaluation/operators/negate.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

	struct negate_visitor : boost::static_visitor<value>
    {
        negate_visitor(unary_operator_context const& context) :
            _context(context)
        {
        }

        result_type operator()(int64_t operand) const
        {
            if (operand == numeric_limits<int64_t>::min()) {
            	throw evaluation_exception((boost::format("negation of %1% results in an arithmetic overflow.") % operand).str(), _context.operand_context());
            }
            return -operand;
        }

        result_type operator()(double operand) const
        {
            return -operand;
        }

        template <typename T>
        result_type operator()(T const& operand) const
        {
        	throw evaluation_exception((boost::format("expected %1% for unary negation operator but found %2%.") % types::numeric::name() % value(operand).get_type()).str(), _context.operand_context());
        }

     private:
        unary_operator_context const& _context;
    };

    value negate::operator()(unary_operator_context const& context) const
    {
        return boost::apply_visitor(negate_visitor(context), context.operand());
    }

}}}}  // namespace puppet::compiler::evaluation::operators
