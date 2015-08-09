#include <puppet/runtime/operators/assignment.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    value assignment::operator()(binary_context& context) const
    {
        auto& evaluator = context.evaluator();

        // Ensure the left-hand side is a variable
        auto var = boost::get<variable>(&context.left());
        if (!var) {
            throw evaluator.create_exception(context.left_position(), (boost::format("cannot assign to %1%: assignment can only be performed on variables.") % get_type(context.left())).str());
        }
        // Ensure the variable isn't a match variable
        if (isdigit(var->name()[0])) {
            throw evaluator.create_exception(context.left_position(), (boost::format("cannot assign to $%1%: the name is reserved as a match variable.") % var->name()).str());
        }
        // Ensure the variable is local to the current scope
        if (var->name().find(':') != string::npos) {
            throw evaluator.create_exception(context.left_position(), (boost::format("cannot assign to $%1%: assignment can only be performed on variables local to the current scope.") % var->name()).str());
        }

        // If the right side is a variable, assign to the variable's value
        shared_ptr<values::value const> value;
        if (auto existing = as<variable>(context.right())) {
            value = existing->value_ptr();
        } else {
            value = make_shared<values::value const>(rvalue_cast(context.right()));
        }

        // Assign the existing value
        auto previous = evaluator.context().current_scope()->set(var->name(), rvalue_cast(value), evaluator.path(), context.left_position().line());
        if (previous) {
            if (previous->path() && !previous->path()->empty()) {
                throw evaluator.create_exception(context.left_position(), (boost::format("cannot assign to $%1%: variable was previously assigned at %2%:%3%.") % var->name() % *previous->path() % previous->line()).str());
            }
            throw evaluator.create_exception(context.left_position(), (boost::format("cannot assign to $%1%: a fact or node parameter exists with the same name.") % var->name()).str());
        }

        // Update the assigned variable's value
        var->assign(value);
        return rvalue_cast(context.left());
    }

}}}  // namespace puppet::runtime::operators
