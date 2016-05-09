#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    call_context::call_context(evaluation::context& context, ast::function_call_expression const& expression) :
        _context(context),
        _name(expression.function),
        _block(expression.lambda)
    {
        // Capture the closure scope if there is a block
        if (_block) {
            _closure_scope = context.current_scope();
        }

        _arguments.reserve(expression.arguments.size());

        evaluate_arguments(expression.arguments);
    }

    call_context::call_context(evaluation::context& context, ast::method_call_expression const& expression, values::value& instance, ast::context const& instance_context, bool splat) :
        _context(context),
        _name(expression.method),
        _block(expression.lambda)
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

        evaluate_arguments(expression.arguments);
    }

    call_context::call_context(evaluation::context& context, ast::new_expression const& expression, ast::name const& name) :
        _context(context),
        _name(name),
        _block(expression.lambda)
    {
        _arguments.reserve(expression.arguments.size() + 1);

        // Push back the type as the first argument
        evaluation::evaluator evaluator{ _context };
        _argument_contexts.emplace_back(expression.type.context());
        _arguments.emplace_back(evaluator.evaluate(expression.type));

        evaluate_arguments(expression.arguments);
    }

    evaluation::context& call_context::context() const
    {
        return _context;
    }

    ast::name const& call_context::name() const
    {
        return _name;
    }

    values::array& call_context::arguments()
    {
        return _arguments;
    }

    values::array const& call_context::arguments() const
    {
        return _arguments;
    }

    values::value& call_context::argument(size_t index)
    {
        return _arguments.at(index);
    }

    values::value const& call_context::argument(size_t index) const
    {
        return _arguments.at(index);
    }

    ast::context const& call_context::argument_context(size_t index) const
    {
        return _argument_contexts.at(index);
    }

    boost::optional<ast::lambda_expression> const& call_context::block() const
    {
        return _block;
    }

    values::value call_context::yield(values::array& arguments) const
    {
        try {
            return yield_without_catch(arguments);
        } catch (argument_exception const& ex) {
            throw evaluation_exception(ex.what(), _block->parameters[ex.index()].context(), _context.backtrace());
        }
    }

    values::value call_context::yield_without_catch(values::array& arguments) const
    {
        if (!_block) {
            return values::undef();
        }
        function_evaluator evaluator{ _context, "<block>", _block->parameters, _block->body };
        return evaluator.evaluate(arguments, _closure_scope);
    }

    void call_context::evaluate_arguments(vector<ast::expression> const& arguments)
    {
        evaluation::evaluator evaluator{ _context };
        for (auto& argument : arguments) {
            auto value = evaluator.evaluate(argument);

            // If the argument is being splatted, move its elements
            if (argument.is_splat()) {
                auto unfolded = value.to_array();
                _argument_contexts.insert(_argument_contexts.end(), unfolded.size(), argument.context());
                _arguments.reserve(_arguments.size() + unfolded.size());
                _arguments.insert(_arguments.end(), std::make_move_iterator(unfolded.begin()), std::make_move_iterator(unfolded.end()));
                continue;
            }

            _argument_contexts.push_back(argument.context());
            _arguments.emplace_back(rvalue_cast(value));
        }

        // Arguments have evaluated, set the current context to the function's name
        _context.current_context(_name);
    }

}}}}  // namespace puppet::compiler::evaluation::functions
