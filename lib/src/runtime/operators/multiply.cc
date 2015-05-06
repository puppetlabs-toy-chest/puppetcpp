#include <puppet/runtime/operators/multiply.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <boost/format.hpp>
#include <limits>
#include <cfenv>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    struct multiply_visitor : boost::static_visitor<value>
    {
        multiply_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            if (right > 0) {
                if (left > (numeric_limits<int64_t>::max() / right)) {
                    throw evaluation_exception(_left_position, (boost::format("multiplication of %1% and %2% results in an arithmetic overflow.") % left % right).str());
                }
                if (left < (numeric_limits<int64_t>::min() / right)) {
                    throw evaluation_exception(_left_position, (boost::format("multiplication of %1% and %2% results in an arithmetic underflow.") % left % right).str());
                }
            } else if (right < -1) {
                if (left > (numeric_limits<int64_t>::min() / right)) {
                    throw evaluation_exception(_left_position, (boost::format("multiplication of %1% and %2% results in an arithmetic underflow.") % left % right).str());
                }
                if (left < (numeric_limits<int64_t>::max() / right)) {
                    throw evaluation_exception(_left_position, (boost::format("multiplication of %1% and %2% results in an arithmetic overflow.") % left % right).str());
                }
            } else if (right == -1) {
                if (left == numeric_limits<int64_t>::min()) {
                    throw evaluation_exception(_left_position, (boost::format("multiplication of %1% and %2% results in an arithmetic overflow.") % left % right).str());
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
                throw evaluation_exception(_left_position, (boost::format("multiplication of %1% and %2% results in an arithmetic overflow.") % left % right).str());
            } else if (fetestexcept(FE_UNDERFLOW)) {
                throw evaluation_exception(_left_position, (boost::format("multiplication of %1% and %2% results in an arithmetic underflow.") % left % right).str());
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
            throw evaluation_exception(_right_position, (boost::format("expected %1% for arithmetic multiplication but found %2%.") % types::numeric::name() % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected %1% for arithmetic multiplication but found %2%.") % types::numeric::name() % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected %1% for arithmetic multiplication but found %2%.") % types::numeric::name() % get_type(left)).str());
        }

    private:
        token_position const& _left_position;
        token_position const& _right_position;
    };

    value multiply::operator()(binary_context& context) const
    {
        return boost::apply_visitor(multiply_visitor(context.left_position(), context.right_position()), dereference(context.left()), dereference(context.right()));
    }

}}}  // namespace puppet::runtime::operators
