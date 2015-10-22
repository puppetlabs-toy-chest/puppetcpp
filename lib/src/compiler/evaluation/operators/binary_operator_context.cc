#include <puppet/compiler/evaluation/operators/binary_operator_context.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    binary_operator_context::binary_operator_context(evaluation::context& context, value& left, ast::context const& left_context, value& right, ast::context const& right_context) :
        _context(context),
        _left(left),
        _left_context(left_context),
        _right(right),
        _right_context(right_context)
    {
    }

    evaluation::context& binary_operator_context::context() const
    {
        return _context;
    }

    value& binary_operator_context::left() const
    {
        return _left;
    }

    ast::context const& binary_operator_context::left_context() const
    {
        return _left_context;
    }

    value& binary_operator_context::right() const
    {
        return _right;
    }

    ast::context const& binary_operator_context::right_context() const
    {
        return _right_context;
    }

}}}}  // namespace puppet::compiler::evaluation::operators
