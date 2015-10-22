#include <puppet/compiler/evaluation/operators/assignment.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    value assignment::operator()(binary_operator_context const& context) const
    {
        auto& left = context.left();
        auto& left_context = context.left_context();
        auto& right = context.right();

        // Ensure the left-hand side is a variable
        auto var = boost::get<variable>(&left);
        if (!var) {
            throw evaluation_exception((boost::format("cannot assign to %1%: assignment can only be performed on variables.") % left.get_type()).str(), left_context);
        }
        // Ensure the variable isn't a match variable
        if (isdigit(var->name()[0])) {
            throw evaluation_exception((boost::format("cannot assign to $%1%: the name is reserved as a match variable.") % var->name()).str(), left_context);
        }
        // Ensure the variable is local to the current scope
        if (var->name().find(':') != string::npos) {
            throw evaluation_exception((boost::format("cannot assign to $%1%: assignment can only be performed on variables local to the current scope.") % var->name()).str(), left_context);
        }

        // If the right side is a variable, assign to the variable's value
        shared_ptr<runtime::values::value const> value;
        if (auto existing = boost::get<variable>(&right)) {
            value = existing->shared_value();
        } else {
            value = std::make_shared<runtime::values::value const>(rvalue_cast(right));
        }

        // Set the variable in the current scope
        auto previous = context.context().current_scope()->set(var->name(), value, &left_context);
        if (previous) {
            if (previous->tree) {
                throw evaluation_exception((boost::format("cannot assign to $%1%: variable was previously assigned at %2%:%3%.") % var->name() % previous->tree->path() % previous->position.line()).str(), left_context);
            }
            throw evaluation_exception((boost::format("cannot assign to $%1%: a fact or node parameter exists with the same name.") % var->name()).str(), left_context);
        }

        // Update the assigned variable's value
        var->assign(value);
        return rvalue_cast(left);
    }

}}}}  // namespace puppet::compiler::evaluation::operators