#include <puppet/runtime/operators/assignment.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    value assignment::operator()(binary_context& context) const
    {
        // Ensure the left-hand side is a variable
        auto var = boost::get<variable>(&context.left());
        if (!var) {
            throw evaluation_exception(context.left_position(), (boost::format("cannot assign to %1%: assignment can only be performed on variables.") % get_type(context.left())).str());
        }
        // Ensure the variable isn't a match variable
        if (isdigit(var->name()[0])) {
            throw evaluation_exception(context.left_position(), (boost::format("cannot assign to $%1%: variable name is reserved for match variables.") % var->name()).str());
        }
        // Ensure the variable is local to the current scope
        if (var->name().find(':') != string::npos) {
            throw evaluation_exception(context.left_position(), (boost::format("cannot assign to $%1%: assignment can only be performed on variables local to the current scope.") % var->name()).str());
        }

        auto& evaluator = context.evaluator();
        auto& scope = evaluator.context().current_scope();

        // If the right side is a variable, assign to the existing variable
        shared_ptr<values::value const> value;
        assigned_variable const* assigned = nullptr;
        if (auto existing = as<variable>(context.right())) {
            value = existing->value_ptr();
        } else {
            value = make_shared<values::value const>(rvalue_cast(context.right()));
        }

        // Assign the existing value
        assigned = scope->set(var->name(), rvalue_cast(value), evaluator.path(), context.left_position().line());
        if (!assigned) {
            auto previous = scope->get(var->name());
            if (previous && previous->path() && !previous->path()->empty()) {
                throw evaluation_exception(context.left_position(), (boost::format("cannot assign to $%1%: variable was previously assigned at %2%:%3%.") % var->name() % *previous->path() % previous->line()).str());
            }
            throw evaluation_exception(context.left_position(), (boost::format("cannot assign to $%1%: variable was previously assigned.") % var->name()).str());
        }

        // Update the reference's value
        var->assign(assigned->value());
        return rvalue_cast(context.left());
    }

}}}  // namespace puppet::runtime::operators
