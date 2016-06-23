#include <puppet/compiler/evaluation/operators/binary/assignment.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    static void assign(call_context& context, values::value& left, values::value& right);

    static void assign(call_context& context, values::variable& variable, values::value& value)
    {
        auto& evaluation_context = context.context();

        // If the right side is a variable, assign to the variable's value
        shared_ptr<runtime::values::value const> shared_value;
        if (auto other = boost::get<values::variable>(&value)) {
            shared_value = other->shared_value();
        } else {
            shared_value = std::make_shared<runtime::values::value const>(rvalue_cast(value));
        }

        // Set the variable in the current scope
        auto previous = evaluation_context.current_scope()->set(variable.name(), shared_value, context.left_context());
        if (previous) {
            if (previous->path()) {
                throw evaluation_exception(
                    (boost::format("cannot assign to $%1%: variable was previously assigned at %2%:%3%.") %
                     variable.name() %
                     *previous->path() %
                     previous->line()
                    ).str(),
                    context.left_context(),
                    evaluation_context.backtrace()
                );
            }
            throw evaluation_exception(
                (boost::format("cannot assign to $%1%: a fact or node parameter exists with the same name.") %
                 variable.name()
                ).str(),
                context.left_context(),
                evaluation_context.backtrace()
            );
        }

        variable.assign(rvalue_cast(shared_value));
    }

    static void assign(call_context& context, values::array& left, values::hash& right)
    {
        for (auto& element : left) {
            auto variable = boost::get<values::variable>(element.get_ptr());
            if (!variable) {
                throw evaluation_exception(
                    (boost::format("expected assignment to a variable when assigning from a hash but found %1%.") %
                     element->infer_type()
                    ).str(),
                    context.left_context(),
                    context.context().backtrace()
                );
            }

            auto value = right.get(variable->name());
            if (!value) {
                throw evaluation_exception(
                    (boost::format("cannot assign to variable $%1%: a matching key was not found in the hash.") %
                     variable->name()
                    ).str(),
                    context.right_context(),
                    context.context().backtrace()
                );
            }
            assign(context, *variable, *value);
        }
    }

    static void assign(call_context& context, values::array& left, values::array& right)
    {
        // Arrays must be of equal size
        if (left.size() != right.size()) {
            throw evaluation_exception(
                (boost::format("cannot assign to array of size %1% from an array of size %2%.") %
                left.size() %
                 right.size()
                ).str(),
                context.right_context(),
                context.context().backtrace()
            );
        }
        for (size_t i = 0; i < left.size(); ++i) {
            assign(context, left[i], right[i]);
        }
    }

    static void assign(call_context& context, values::array& left, values::value& right)
    {
        if (right.as<values::array>()) {
            auto right_array = right.move_as<values::array>();
            assign(context, left, right_array);
            return;
        }

        if (right.as<values::hash>()) {
            auto right_hash = right.move_as<values::hash>();
            assign(context, left, right_hash);
            return;
        }

        throw evaluation_exception(
            (boost::format("expected %1% or %2% when assigning to %1% but found %3%.") %
             types::array::name() %
             types::hash::name() %
             right.infer_type()
            ).str(),
            context.right_context(),
            context.context().backtrace()
        );
    }

    static void assign(call_context& context, values::value& left, values::value& right)
    {
        if (auto variable = boost::get<values::variable>(&left)) {
            assign(context, *variable, right);
            return;
        }

        if (auto array = boost::get<values::array>(&left)) {
            assign(context, *array, right);
            return;
        }

        throw evaluation_exception(
            (boost::format("cannot assign to %1%: assignment can only be performed on variables and arrays of variables.") %
             left.infer_type()
            ).str(),
            context.left_context(),
            context.context().backtrace()
        );
    }

    descriptor assignment::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::assignment };

        descriptor.add("Any", "Any", [](call_context& context) {
            assign(context, context.left(), context.right());
            return rvalue_cast(context.left());
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
