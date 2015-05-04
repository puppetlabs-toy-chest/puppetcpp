#include <puppet/runtime/operators/modulo.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    struct modulo_visitor : boost::static_visitor<value>
    {
        modulo_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            if (right == 0) {
                throw evaluation_exception(_right_position, "cannot divide by zero.");
            }
            return left % right;
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected %1% for arithmetic modulo but found %2%.") % types::integer::name() % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected %1% for arithmetic modulo but found %2%.") % types::integer::name() % get_type(left)).str());
        }

     private:
        token_position const& _left_position;
        token_position const& _right_position;
    };

    value modulo::operator()(binary_context& context) const
    {
        return boost::apply_visitor(modulo_visitor(context.left_position(), context.right_position()), dereference(context.left()), dereference(context.right()));
    }

}}}  // namespace puppet::runtime::operators
