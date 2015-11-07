#include <puppet/compiler/evaluation/operators/modulo.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    struct modulo_visitor : boost::static_visitor<value>
    {
        explicit modulo_visitor(binary_operator_context const& context) :
            _context(context)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            if (right == 0) {
                throw evaluation_exception("cannot divide by zero.", _context.right_context());
            }
            return left % right;
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception((boost::format("expected %1% for arithmetic modulo but found %2%.") % types::numeric::name() % value(right).get_type()).str(), _context.right_context());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception((boost::format("expected %1% for arithmetic modulo but found %2%.") % types::numeric::name() % value(left).get_type()).str(), _context.left_context());
        }

     private:
        binary_operator_context const& _context;
    };

    value modulo::operator()(binary_operator_context const& context) const
    {
        return boost::apply_visitor(modulo_visitor(context), context.left(), context.right());
    }

}}}}  // namespace puppet::compiler::evaluation::operators
