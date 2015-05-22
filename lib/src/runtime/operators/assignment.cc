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
        if (var->match()) {
            throw evaluation_exception(context.left_position(), (boost::format("cannot assign to $%1%: variable name is reserved for match variables.") % var->name()).str());
        }
        // Ensure the variable is local to the current scope
        if (var->name().find(':') != string::npos) {
            throw evaluation_exception(context.left_position(), (boost::format("cannot assign to $%1%: assignment can only be performed on variables local to the current scope.") % var->name()).str());
        }

        // If the right-hand side is a match variable, copy the value because it is inherently ephemeral
        auto var_right = boost::get<variable>(&context.right());
        if (var_right && var_right->match()) {
            context.right() = var_right->value();
        }

        // Set the value in the current scope
        auto value = context.evaluator().scope().set(var->name(), rvalue_cast(context.right()), context.left_position().line());
        if (!value) {
            // Check where the variable was previously assigned
            auto line = context.evaluator().scope().where(var->name());
            if (line) {
                throw evaluation_exception(context.left_position(), (boost::format("cannot assign to $%1%: variable was previously assigned at %2%:%3%.") % var->name() % context.evaluator().path() % line).str());
            }
            // Assume it's a fact if we don't know where it was assigned
            throw evaluation_exception(context.left_position(), (boost::format("cannot assign to $%1%: a fact of the same name already exists.") % var->name()).str());
        }

        // Update the variable's value and return the variable reference
        var->update(value);
        return rvalue_cast(context.left());
    }

}}}  // namespace puppet::runtime::operators
