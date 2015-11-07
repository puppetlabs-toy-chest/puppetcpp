#include <puppet/compiler/evaluation/operators/left_shift.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    struct left_shift_visitor : boost::static_visitor<value>
    {
        explicit left_shift_visitor(binary_operator_context const& context) :
            _context(context)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            // If right < 0. reverse direction
            // If left is negative, keep the sign bit
            if (right < 0 && left < 0) {
                return -(-left >> -right);
            }
            if (right < 0) {
                return left >> -right;
            }
            if (left < 0) {
                return -(-left << right);
            }
            return left << right;
        }

        template <typename Right>
        result_type operator()(values::array const& left, Right const& right) const
        {
            auto copy = left;
            copy.push_back(right);
            return copy;
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception((boost::format("expected %1% for bitwise left shift but found %2%.") % types::integer::name() % value(right).get_type()).str(), _context.right_context());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception((boost::format("expected %1% for bitwise left shift but found %2%.") % types::integer::name() % value(left).get_type()).str(), _context.left_context());
        }

     private:
        binary_operator_context const& _context;
    };

    value left_shift::operator()(binary_operator_context const& context) const
    {
        // Prepare the left and right for mutation
        return boost::apply_visitor(left_shift_visitor(context), context.left(), context.right());
    }

}}}}  // namespace puppet::compiler::evaluation::operators
