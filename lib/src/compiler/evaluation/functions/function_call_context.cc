#include <puppet/compiler/evaluation/functions/function_call_context.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    function_call_context::function_call_context(evaluation::context& context, ast::function_call_expression const& expression) :
        _context(context),
        _name(expression.function),
        _lambda(expression.lambda)
    {
        _arguments.reserve(expression.arguments.size());

        // Evaluate the arguments
        evaluate_arguments(expression.arguments);
    }

    function_call_context::function_call_context(evaluation::context& context, ast::method_call_expression const& expression, values::value& instance, ast::context const& instance_context, bool splat) :
        _context(context),
        _name(expression.method),
        _lambda(expression.lambda)
    {
        _arguments.reserve(expression.arguments.size() + 1);

        // Check if the instance itself is being splatted
        if (splat && instance.as<values::array>()) {
            auto unfolded = instance.move_as<values::array>();
            _argument_contexts.insert(_argument_contexts.end(), unfolded.size(), instance_context);
            _arguments.reserve(_arguments.size() + unfolded.size());
            _arguments.insert(_arguments.end(), std::make_move_iterator(unfolded.begin()), std::make_move_iterator(unfolded.end()));
        } else {
            _argument_contexts.push_back(instance_context);
            _arguments.emplace_back(rvalue_cast(instance));
        }

        // Evaluate the arguments
        evaluate_arguments(expression.arguments);
    }

    evaluation::context& function_call_context::context() const
    {
        return _context;
    }

    ast::name const& function_call_context::name() const
    {
        return _name;
    }

    values::array& function_call_context::arguments()
    {
        return _arguments;
    }

    values::value& function_call_context::argument(size_t index)
    {
        return _arguments.at(index);
    }

    ast::context const& function_call_context::argument_context(size_t index) const
    {
        return _argument_contexts.at(index);
    }

    boost::optional<ast::lambda_expression> const& function_call_context::lambda() const
    {
        return _lambda;
    }

    values::value function_call_context::yield(values::array& arguments) const
    {
        // Execute the lambda
        try {
            return yield_without_catch(arguments);
        } catch (argument_exception const& ex) {
            throw evaluation_exception(ex.what(), _lambda->parameters[ex.index()].context());
        }
    }

    values::value function_call_context::yield_without_catch(values::array& arguments) const
    {
        if (!_lambda) {
            return values::undef();
        }
        call_evaluator evaluator { _context, _lambda->parameters, _lambda->body };
        return evaluator.evaluate(arguments);
    }

    void function_call_context::evaluate_arguments(vector<ast::expression> const& arguments)
    {
        // Evaluate the arguments
        evaluation::evaluator evaluator{ _context };
        for (auto& argument : arguments) {
            auto value = evaluator.evaluate(argument);

            // If the argument is being splatted, move its elements
            if (argument.is_splat() && value.as<values::array>()) {
                auto unfolded = value.move_as<values::array>();
                _argument_contexts.insert(_argument_contexts.end(), unfolded.size(), argument.context());
                _arguments.reserve(_arguments.size() + unfolded.size());
                _arguments.insert(_arguments.end(), std::make_move_iterator(unfolded.begin()), std::make_move_iterator(unfolded.end()));
                continue;
            }

            _argument_contexts.push_back(argument.context());
            _arguments.emplace_back(rvalue_cast(value));
        }
    }

}}}}  // namespace puppet::compiler::evaluation::functions
