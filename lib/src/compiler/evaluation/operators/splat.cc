#include <puppet/compiler/evaluation/operators/splat.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

 	value splat::operator()(unary_operator_context const& context) const
    {
        return context.operand().to_array();
    }

}}}}  // namespace puppet::compiler::evaluation::operators
