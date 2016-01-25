#include <puppet/compiler/evaluation/operators/unary/negate.hpp>
#include <puppet/compiler/evaluation/operators/unary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace unary {

    descriptor negate::create_descriptor()
    {
        unary::descriptor descriptor{ ast::unary_operator::negate };

        descriptor.add("Integer", [](call_context& context) {
            auto operand = context.operand().require<int64_t>();
            if (operand == numeric_limits<int64_t>::min()) {
                throw evaluation_exception((boost::format("negation of %1% results in an arithmetic overflow.") % operand).str(), context.operand_context());
            }
            return -operand;
        });
        descriptor.add("Float", [](call_context& context) {
            return -context.operand().require<double>();
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::unary
