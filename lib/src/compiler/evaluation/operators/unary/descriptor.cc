#include <puppet/compiler/evaluation/operators/unary/descriptor.hpp>
#include <puppet/compiler/evaluation/operators/unary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace unary {

    descriptor::descriptor(ast::unary_operator oper) :
        _operator(oper)
    {
    }

    ast::unary_operator descriptor::oper() const
    {
        return _operator;
    }

    bool descriptor::dispatchable() const
    {
        return !_dispatch_descriptors.empty();
    }

    void descriptor::add(string const& type, callback_type callback)
    {
        auto operand_type = values::type::parse(type);
        if (!operand_type) {
            throw runtime_error((boost::format("unary operator '%1%' cannot add an overload with invalid operand type '%2%'.") % _operator % type).str());
        }

        dispatch_descriptor descriptor;
        descriptor.type = rvalue_cast(*operand_type);
        descriptor.callback = rvalue_cast(callback);
        _dispatch_descriptors.emplace_back(rvalue_cast(descriptor));
    }

    values::value descriptor::dispatch(call_context& context) const
    {
        // Search for a dispatch descriptor with a matching operand type
        // TODO: in the future, this should dispatch to the most specific overload rather than the first dispatchable overload
        for (auto& descriptor : _dispatch_descriptors) {
            if (descriptor.type.is_instance(context.operand())) {
                return descriptor.callback(context);
            }
        }

        // Build a set of expected types
        values::type_set set;
        for (auto& descriptor : _dispatch_descriptors) {
            set.add(descriptor.type);
        }

        auto& evaluation_context = context.context();
        if (set.empty()) {
            // Generic error for no matched types
            throw evaluation_exception(
                (boost::format("unary operator '%1%' cannot be dispatched.") %
                 _operator
                ).str(),
                context.operator_context(),
                evaluation_context.backtrace()
            );
        }

        throw evaluation_exception(
            (boost::format("unary operator '%1%' expects %2% but was given %3%.") %
             _operator %
             set %
             context.operand().get_type()
            ).str(),
            context.operand_context(),
            evaluation_context.backtrace()
        );
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::unary
