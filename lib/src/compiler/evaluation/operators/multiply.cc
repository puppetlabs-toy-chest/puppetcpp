#include <puppet/compiler/evaluation/operators/multiply.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <limits>
#include <cfenv>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    struct multiply_visitor : boost::static_visitor<value>
    {
        explicit multiply_visitor(binary_operator_context const& context) :
            _context(context)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            if (right > 0) {
                if (left > (numeric_limits<int64_t>::max() / right)) {
                    throw evaluation_exception((boost::format("multiplication of %1% and %2% results in an arithmetic overflow.") % left % right).str(), _context.right_context());
                }
                if (left < (numeric_limits<int64_t>::min() / right)) {
                    throw evaluation_exception((boost::format("multiplication of %1% and %2% results in an arithmetic underflow.") % left % right).str(), _context.right_context());
                }
            } else if (right < -1) {
                if (left > (numeric_limits<int64_t>::min() / right)) {
                    throw evaluation_exception((boost::format("multiplication of %1% and %2% results in an arithmetic underflow.") % left % right).str(), _context.right_context());
                }
                if (left < (numeric_limits<int64_t>::max() / right)) {
                    throw evaluation_exception((boost::format("multiplication of %1% and %2% results in an arithmetic overflow.") % left % right).str(), _context.right_context());
                }
            } else if (right == -1) {
                if (left == numeric_limits<int64_t>::min()) {
                    throw evaluation_exception((boost::format("multiplication of %1% and %2% results in an arithmetic overflow.") % left % right).str(), _context.right_context());
                }
            }
            return left * right;
        }

        result_type operator()(int64_t left, long double right) const
        {
            return operator()(static_cast<long double>(left), right);
        }

        result_type operator()(long double left, int64_t right) const
        {
            return operator()(left, static_cast<long double>(right));
        }

        result_type operator()(long double left, long double right) const
        {
            feclearexcept(FE_OVERFLOW | FE_UNDERFLOW);
            long double result = left * right;
            if (fetestexcept(FE_OVERFLOW)) {
                throw evaluation_exception((boost::format("multiplication of %1% and %2% results in an arithmetic overflow.") % left % right).str(), _context.right_context());
            } else if (fetestexcept(FE_UNDERFLOW)) {
                throw evaluation_exception((boost::format("multiplication of %1% and %2% results in an arithmetic underflow.") % left % right).str(), _context.right_context());
            }
            return result;
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception((boost::format("expected %1% for arithmetic multiplication but found %2%.") % types::numeric::name() % value(right).get_type()).str(), _context.right_context());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw evaluation_exception((boost::format("expected %1% for arithmetic multiplication but found %2%.") % types::numeric::name() % value(right).get_type()).str(), _context.right_context());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception((boost::format("expected %1% for arithmetic multiplication but found %2%.") % types::numeric::name() % value(left).get_type()).str(), _context.left_context());
        }

     private:
        binary_operator_context const& _context;
    };

    value multiply::operator()(binary_operator_context const& context) const
    {
        multiply_visitor visitor(context);
        return boost::apply_visitor(visitor, context.left(), context.right());
    }

}}}}  // namespace puppet::compiler::evaluation::operators
