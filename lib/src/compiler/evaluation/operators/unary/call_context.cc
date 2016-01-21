#include <puppet/compiler/evaluation/operators/unary/call_context.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace unary {

    call_context::call_context(
        evaluation::context& context,
        ast::unary_operator oper,
        ast::context const& operator_context,
        value& operand,
        ast::context const& operand_context) :
            _context(context),
            _operator(oper),
            _operator_context(operator_context),
            _operand(operand),
            _operand_context(operand_context)
    {
    }

    evaluation::context& call_context::context() const
    {
        return _context;
    }

    ast::unary_operator call_context::oper() const
    {
        return _operator;
    }

    ast::context const& call_context::operator_context() const
    {
        return _operator_context;
    }

    value& call_context::operand() const
    {
        return _operand;
    }

    ast::context const& call_context::operand_context() const
    {
        return _operand_context;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::unary
