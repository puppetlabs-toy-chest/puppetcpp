#include <puppet/compiler/evaluation/operators/binary/descriptor.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    descriptor::descriptor(ast::binary_operator oper) :
        _operator(oper)
    {
    }

    ast::binary_operator descriptor::oper() const
    {
        return _operator;
    }

    bool descriptor::dispatchable() const
    {
        return !_dispatch_descriptors.empty();
    }

    void descriptor::add(string const& left_type, string const& right_type, callback_type callback)
    {
        auto left = values::type::parse(left_type);
        if (!left) {
            throw runtime_error((boost::format("binary operator '%1%' cannot add an overload with invalid left operand type '%2%'.") % _operator % left_type).str());
        }

        auto right = values::type::parse(right_type);
        if (!right) {
            throw runtime_error((boost::format("binary operator '%1%' cannot add an overload with invalid right operand type '%2%'.") % _operator % right_type).str());
        }

        dispatch_descriptor descriptor;
        descriptor.left_type = rvalue_cast(*left);
        descriptor.right_type = rvalue_cast(*right);
        descriptor.callback = rvalue_cast(callback);
        _dispatch_descriptors.emplace_back(rvalue_cast(descriptor));
    }

    values::value descriptor::dispatch(call_context& context) const
    {

        // Search for a dispatch descriptor with the matching left and right types
        // TODO: in the future, this should dispatch to the most specific overload rather than the first dispatchable overload
        types::recursion_guard guard;
        for (auto& descriptor : _dispatch_descriptors) {
            if (descriptor.left_type.is_instance(context.left(), guard) && descriptor.right_type.is_instance(context.right(), guard)) {
                return descriptor.callback(context);
            }
        }

        // Check to see if the LHS or RHS had at least one matching type; if so, build a set of expected types for the other side
        values::type_set set;
        values::type const* matched_type = nullptr;
        bool rhs_match = true;

        for (auto& descriptor : _dispatch_descriptors) {
            // Check to see if the LHS matches
            if (!matched_type && descriptor.left_type.is_instance(context.left(), guard)) {
                rhs_match = false;
                matched_type = &descriptor.left_type;
                set.clear();
                set.add(descriptor.right_type);
                continue;
            }
            // Check to see if the RHS matched
            if (!matched_type && descriptor.right_type.is_instance(context.right(), guard)) {
                matched_type = &descriptor.right_type;
                set.clear();
                set.add(descriptor.left_type);
                continue;
            }

            // If nothing matched yet, assume the problem is on the LHS (no matches will add all LHS types)
            if (!matched_type) {
                set.add(descriptor.left_type);
                continue;
            }
            if (!rhs_match) {
                // Ignore the RHS if the LHS doesn't match
                if (*matched_type != descriptor.left_type) {
                    continue;
                }
                set.add(descriptor.right_type);
            } else {
                // Ignore the LHS if the RHS doesn't match
                if (*matched_type != descriptor.right_type) {
                    continue;
                }
                set.add(descriptor.left_type);
            }
        }

        auto& evaluation_context = context.context();
        if (set.empty()) {
            // Generic error for no matched types
            throw evaluation_exception(
                (boost::format("binary operator '%1%' cannot be dispatched.") %
                 _operator
                ).str(),
                context.operator_context(),
                evaluation_context.backtrace()
            );
        }

        throw evaluation_exception(
            (boost::format("binary operator '%1%' expects %2% but was given %3%.") %
             _operator %
             set %
             (!rhs_match ? context.right() : context.left()).get_type()
            ).str(),
            !rhs_match ? context.right_context() : context.left_context(),
            evaluation_context.backtrace()
        );
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
