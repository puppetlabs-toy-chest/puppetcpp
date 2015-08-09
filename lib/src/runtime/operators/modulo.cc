#include <puppet/runtime/operators/modulo.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    struct modulo_visitor : boost::static_visitor<value>
    {
        explicit modulo_visitor(binary_context& context) :
            _context(context)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            if (right == 0) {
                throw _context.evaluator().create_exception(_context.right_position(), "cannot divide by zero.");
            }
            return left % right;
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw _context.evaluator().create_exception(_context.right_position(), (boost::format("expected %1% for arithmetic modulo but found %2%.") % types::integer::name() % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw _context.evaluator().create_exception(_context.left_position(), (boost::format("expected %1% for arithmetic modulo but found %2%.") % types::integer::name() % get_type(left)).str());
        }

     private:
        binary_context& _context;
    };

    value modulo::operator()(binary_context& context) const
    {
        modulo_visitor visitor(context);
        return boost::apply_visitor(visitor, dereference(context.left()), dereference(context.right()));
    }

}}}  // namespace puppet::runtime::operators
