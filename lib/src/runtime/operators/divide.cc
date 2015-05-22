#include <puppet/runtime/operators/divide.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <boost/format.hpp>
#include <limits>
#include <cfenv>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    struct divide_visitor : boost::static_visitor<value>
    {
        divide_visitor(lexer::position const& left_position, lexer::position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            if (right == 0) {
                throw evaluation_exception(_right_position, "cannot divide by zero.");
            }
            if (left == numeric_limits<int64_t>::min() && right == -1) {
                throw evaluation_exception(_left_position, (boost::format("division of %1% by %2% results in an arithmetic overflow.") % left % right).str());
            }
            return left / right;
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
            feclearexcept(FE_OVERFLOW | FE_UNDERFLOW | FE_DIVBYZERO);
            long double result = left / right;
            if (fetestexcept(FE_DIVBYZERO)) {
                throw evaluation_exception(_right_position, "cannot divide by zero.");
            } else if (fetestexcept(FE_OVERFLOW)) {
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
            throw evaluation_exception(_right_position, (boost::format("expected %1% for arithmetic division but found %2%.") % types::numeric::name() % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected %1% for arithmetic division but found %2%.") % types::numeric::name() % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected %1% for arithmetic division but found %2%.") % types::numeric::name() % get_type(left)).str());
        }

    private:
        lexer::position const& _left_position;
        lexer::position const& _right_position;
    };

    value divide::operator()(binary_context& context) const
    {
        return boost::apply_visitor(divide_visitor(context.left_position(), context.right_position()), dereference(context.left()), dereference(context.right()));
    }

}}}  // namespace puppet::runtime::operators
