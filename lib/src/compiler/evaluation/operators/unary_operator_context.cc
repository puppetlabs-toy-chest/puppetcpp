#include <puppet/compiler/evaluation/operators/unary_operator_context.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    unary_operator_context::unary_operator_context(evaluation::context& context, value& operand, ast::context const& operand_context) :
        _context(context),
        _operand(operand),
        _operand_context(operand_context)
    {
    }

    evaluation::context& unary_operator_context::context() const
    {
        return _context;
    }

    value& unary_operator_context::operand() const
    {
        return _operand;
    }

    ast::context const& unary_operator_context::operand_context() const
    {
        return _operand_context;
    }

}}}}  // namespace puppet::compiler::evaluation::operators
