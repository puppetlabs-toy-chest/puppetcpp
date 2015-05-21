#include <puppet/runtime/dispatcher.hpp>
#include <puppet/runtime/functions/assert_type.hpp>
#include <puppet/runtime/functions/each.hpp>
#include <puppet/runtime/functions/fail.hpp>
#include <puppet/runtime/functions/filter.hpp>
#include <puppet/runtime/functions/logging.hpp>
#include <puppet/runtime/functions/split.hpp>
#include <puppet/runtime/functions/with.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;
using boost::optional;

namespace puppet { namespace runtime {

    call_context::call_context(
        expression_evaluator& evaluator,
        string const& name,
        lexer::token_position const& position,
        optional<vector<ast::expression>> const& arguments,
        optional<ast::lambda> const& lambda,
        value* first_value,
        ast::primary_expression const* first_expression,
        lexer::token_position const* first_position) :
            _evaluator(evaluator),
            _name(name),
            _position(position),
            _yielder(evaluator, _position, lambda)
    {
        _arguments.reserve((arguments ? arguments->size() : 0) + (first_value ? 1 : 0));
        if (first_value) {
            // If this is the first expression, attempt to unfold a splat
            bool added = false;
            if (first_expression) {
                auto unfold_array = evaluator.unfold(*first_expression, *first_value);
                if (unfold_array) {
                    _positions.insert(_positions.end(), unfold_array->size(), *first_position);
                    _arguments.reserve(_arguments.size() + unfold_array->size());
                    _arguments.insert(_arguments.end(), std::make_move_iterator(unfold_array->begin()), std::make_move_iterator(unfold_array->end()));
                    added = true;
                }
            }
            // Add the first argument if nothing was added above
            if (!added) {
                _arguments.emplace_back(rvalue_cast(*first_value));
                _positions.push_back(first_position ? *first_position : position);
            }
        }

        // Evaluate the arguments
        if (arguments) {
            for (auto& expression : *arguments) {
                auto argument = evaluator.evaluate(expression);
                auto unfold_array = evaluator.unfold(expression, argument);

                // If unfolding, append the array to the arguments
                if (unfold_array) {
                    _positions.insert(_positions.end(), unfold_array->size(), expression.position());
                    _arguments.reserve(_arguments.size() + unfold_array->size());
                    _arguments.insert(_arguments.end(), std::make_move_iterator(unfold_array->begin()), std::make_move_iterator(unfold_array->end()));
                    continue;
                }
                _positions.push_back(expression.position());
                _arguments.emplace_back(rvalue_cast(argument));
            }
        }
    }

    expression_evaluator& call_context::evaluator()
    {
        return _evaluator;
    }

    string const& call_context::name() const
    {
        return _name;
    }

    token_position const& call_context::position() const
    {
        return _position;
    }

    token_position const& call_context::position(size_t index) const
    {
        if (index >= _positions.size()) {
            throw runtime_error("argument index out of range.");
        }
        return _positions[index];
    }

    values::array const& call_context::arguments() const
    {
        return _arguments;
    }

    values::array& call_context::arguments()
    {
        return _arguments;
    }

    runtime::yielder const& call_context::yielder() const
    {
        return _yielder;
    }

    runtime::yielder& call_context::yielder()
    {
        return _yielder;
    }

    dispatcher::dispatcher(string const& name, token_position const& position) :
        _name(name),
        _position(position)
    {
        // Keep in alphabetical order
        static const unordered_map<string, function_type> functions {
            { "alert",          functions::logging_function(logging::level::alert) },
            { "assert_type",    functions::assert_type() },
            { "crit",           functions::logging_function(logging::level::critical) },
            { "debug",          functions::logging_function(logging::level::debug) },
            { "each",           functions::each() },
            { "emerg",          functions::logging_function(logging::level::emergency) },
            { "err",            functions::logging_function(logging::level::error) },
            { "fail",           functions::fail() },
            { "filter",         functions::filter() },
            { "info",           functions::logging_function(logging::level::info) },
            { "notice",         functions::logging_function(logging::level::notice) },
            { "split",          functions::split() },
            { "warning",        functions::logging_function(logging::level::warning) },
            { "with",           functions::with() },
        };

        // Find the function
        auto it = functions.find(_name);
        if (it == functions.end()) {
            throw evaluation_exception(_position, (boost::format("unknown function \"%1%\".") % _name).str());
        }
        _function = &it->second;
    }

    value dispatcher::dispatch(
        expression_evaluator& evaluator,
        optional<vector<ast::expression>> const& arguments,
        optional<ast::lambda> const& lambda,
        value* first_value,
        ast::primary_expression const* first_expression,
        lexer::token_position const* first_position) const
    {
        // Dispatch the call
        call_context ctx(evaluator, _name, _position, arguments, lambda, first_value, first_expression, first_position);
        return (*_function)(ctx);
    }

    string const& dispatcher::name() const
    {
        return _name;
    }

    token_position const& dispatcher::position() const
    {
        return _position;
    }

}}  // namespace puppet::runtime
