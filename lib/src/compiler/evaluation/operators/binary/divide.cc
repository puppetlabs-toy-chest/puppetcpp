#include <puppet/compiler/evaluation/operators/binary/divide.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <limits>
#include <cfenv>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    static value divide_by(call_context& context, int64_t left, int64_t right)
    {
        auto& evaluation_context = context.context();

        if (right == 0) {
            throw evaluation_exception("cannot divide by zero.", context.right_context(), evaluation_context.backtrace());
        }
        if (left == numeric_limits<int64_t>::min() && right == -1) {
            throw evaluation_exception(
                (boost::format("division of %1% by %2% results in an arithmetic overflow.") %
                 left %
                 right
                ).str(),
                context.right_context(),
                evaluation_context.backtrace()
            );
        }
        return left / right;
    }

    static value divide_by(call_context& context, double left, double right)
    {
        auto& evaluation_context = context.context();

        feclearexcept(FE_OVERFLOW | FE_UNDERFLOW | FE_DIVBYZERO);
        double result = left / right;
        if (fetestexcept(FE_DIVBYZERO)) {
            throw evaluation_exception("cannot divide by zero.", context.right_context(), evaluation_context.backtrace());
        } else if (fetestexcept(FE_OVERFLOW)) {
            throw evaluation_exception(
                (boost::format("division of %1% and %2% results in an arithmetic overflow.") %
                 left %
                 right
                ).str(),
                context.right_context(),
                evaluation_context.backtrace()
            );
        } else if (fetestexcept(FE_UNDERFLOW)) {
            throw evaluation_exception(
                (boost::format("division of %1% and %2% results in an arithmetic underflow.") %
                 left %
                 right
                ).str(),
                context.right_context(),
                evaluation_context.backtrace()
            );
        }
        return result;
    }

    descriptor divide::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::divide };

        descriptor.add("Integer", "Integer", [](call_context& context) {
            return divide_by(context, context.left().require<int64_t>(), context.right().require<int64_t>());
        });
        descriptor.add("Integer", "Float", [](call_context& context) {
            return divide_by(context, static_cast<double>(context.left().require<int64_t>()), context.right().require<double>());
        });
        descriptor.add("Float", "Integer", [](call_context& context) {
            return divide_by(context, context.left().require<double>(), static_cast<double>(context.right().require<int64_t>()));
        });
        descriptor.add("Float", "Float", [](call_context& context) {
            return divide_by(context, context.left().require<double>(), context.right().require<double>());
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
