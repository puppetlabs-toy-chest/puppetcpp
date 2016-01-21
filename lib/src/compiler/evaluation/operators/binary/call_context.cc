#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    call_context::call_context(
        evaluation::context& context,
        ast::binary_operator oper,
        ast::context const& operator_context,
        value& left,
        ast::context const& left_context,
        value& right,
        ast::context const& right_context) :
            _context(context),
            _operator(oper),
            _operator_context(operator_context),
            _left(left),
            _left_context(left_context),
            _right(right),
            _right_context(right_context)
    {
    }

    evaluation::context& call_context::context() const
    {
        return _context;
    }

    ast::binary_operator call_context::oper() const
    {
        return _operator;
    }

    ast::context const& call_context::operator_context() const
    {
        return _operator_context;
    }

    value& call_context::left() const
    {
        return _left;
    }

    ast::context const& call_context::left_context() const
    {
        return _left_context;
    }

    value& call_context::right() const
    {
        return _right;
    }

    ast::context const& call_context::right_context() const
    {
        return _right_context;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
