#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/runtime/operators/assignment.hpp>
#include <puppet/runtime/operators/divide.hpp>
#include <puppet/runtime/operators/equals.hpp>
#include <puppet/runtime/operators/greater.hpp>
#include <puppet/runtime/operators/greater_equal.hpp>
#include <puppet/runtime/operators/in.hpp>
#include <puppet/runtime/operators/left_shift.hpp>
#include <puppet/runtime/operators/less.hpp>
#include <puppet/runtime/operators/less_equal.hpp>
#include <puppet/runtime/operators/logical_and.hpp>
#include <puppet/runtime/operators/logical_not.hpp>
#include <puppet/runtime/operators/logical_or.hpp>
#include <puppet/runtime/operators/match.hpp>
#include <puppet/runtime/operators/minus.hpp>
#include <puppet/runtime/operators/modulo.hpp>
#include <puppet/runtime/operators/multiply.hpp>
#include <puppet/runtime/operators/negate.hpp>
#include <puppet/runtime/operators/not_equals.hpp>
#include <puppet/runtime/operators/not_match.hpp>
#include <puppet/runtime/operators/plus.hpp>
#include <puppet/runtime/operators/right_shift.hpp>
#include <puppet/runtime/operators/splat.hpp>
#include <puppet/runtime/string_interpolator.hpp>
#include <puppet/runtime/dispatcher.hpp>
#include <puppet/ast/expression_def.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    static bool is_match(context& ctx, value& result, token_position const& result_position, value& expected, token_position const& expected_position)
    {
        // If the expected value is a regex, use match
        auto regex = as<values::regex>(expected);
        if (regex) {
            // Only match against strings
            if (as<string>(result)) {
                operators::binary_context context(ctx, result, result_position, expected, expected_position);
                if (is_truthy(operators::match()(context))) {
                    return true;
                }
            }
            return false;
        }

        // Otherwise, use equals
        return equals(result, expected);
    }

    evaluation_exception::evaluation_exception(token_position position, string const& message) :
        runtime_error(message),
        _position(std::move(position))
    {
    }

    token_position const& evaluation_exception::position() const
    {
        return _position;
    }

    struct basic_expression_visitor : boost::static_visitor<value>
    {
        explicit basic_expression_visitor(expression_evaluator& evaluator) :
            _evaluator(evaluator)
        {
        }

        result_type operator()(ast::undef const&)
        {
            return value();
        }

        result_type operator()(ast::defaulted const&)
        {
            return defaulted();
        }

        result_type operator()(ast::boolean const& boolean)
        {
            return boolean.value();
        }

        result_type operator()(int64_t integer)
        {
            return integer;
        }

        result_type operator()(long double floating)
        {
            return floating;
        }

        result_type operator()(ast::number const& number)
        {
            return boost::apply_visitor(*this, number.value());
        }

        result_type operator()(ast::string const& str)
        {
            string_interpolator interpolator(_evaluator);
            return interpolator.interpolate(str.position(), str.value(), str.escapes(), str.quote(), str.interpolated(), str.margin(), str.remove_break());
        }

        result_type operator()(ast::regex const& regx)
        {
            try {
                return values::regex(regx.value());
            } catch (std::regex_error const& ex) {
                throw evaluation_exception(regx.position(), ex.what());
            }
        }

        result_type operator()(ast::variable const& var)
        {
            static const std::regex match_variable_patterh("^\\d+$");

            auto& name = var.name();

            bool match = false;
            value const* val = nullptr;
            if (regex_match(name, match_variable_patterh)) {
                // Check for invalid match name
                if (name.size() > 1 && name[0] == '0') {
                    throw evaluation_exception(var.position(), (boost::format("variable name $%1% is not a valid match variable name.") % var.name()).str());
                }
                // Look up the match
                val = _evaluator.context().current().get(stoi(name));
                match = true;
            } else {
                val = _evaluator.context().lookup(name);
            }
            return variable(name, val, match);
        }

        result_type operator()(ast::name const& name)
        {
            // Treat as a string
            return name.value();
        }

        result_type operator()(ast::bare_word const& word)
        {
            // Treat as a string
            return word.value();
        }

        result_type operator()(ast::type const& type)
        {
            static const unordered_map<string, values::type> names = {
                { types::any::name(),           types::any() },
                { types::array::name(),         types::array() },
                { types::boolean::name(),       types::boolean() },
                { types::callable::name(),      types::callable() },
                { types::catalog_entry::name(), types::catalog_entry() },
                { types::collection::name(),    types::collection() },
                { types::data::name(),          types::data() },
                { types::defaulted::name(),     types::defaulted() },
                { types::enumeration::name(),   types::enumeration() },
                { types::floating::name(),      types::floating() },
                { types::hash::name(),          types::hash() },
                { types::integer::name(),       types::integer() },
                { types::klass::name(),         types::klass() },
                { types::numeric::name(),       types::numeric() },
                { types::optional::name(),      types::optional(boost::none) },
                { types::pattern::name(),       types::pattern() },
                { types::regexp::name(),        types::regexp() },
                { types::resource::name(),      types::resource() },
                { types::runtime::name(),       types::runtime() },
                { types::scalar::name(),        types::scalar() },
                { types::string::name(),        types::string() },
                { types::structure::name(),     types::structure() },
                { types::tuple::name(),         types::tuple() },
                { types::type::name(),          types::type(boost::none) },
                { types::undef::name(),         types::undef() },
                { types::variant::name(),       types::variant() },
            };

            auto it = names.find(type.name());
            if (it == names.end()) {
                // Assume the unknown type is a resource
                return types::resource(type.name());
            }
            return it->second;
        }

        result_type operator()(ast::array const& array)
        {
            values::array new_array;

            if (array.elements()) {
                for (auto& element : *array.elements()) {
                    auto result = _evaluator.evaluate(element);

                    // If unfolding, append the array's elements
                    auto unfold_array = _evaluator.unfold(element, result);
                    if (unfold_array) {
                        new_array.reserve(new_array.size() + unfold_array->size());
                        new_array.insert(new_array.end(), std::make_move_iterator(unfold_array->begin()), std::make_move_iterator(unfold_array->end()));
                        continue;
                    }
                    new_array.emplace_back(std::move(result));
                }
            }
            return new_array;
        }

        result_type operator()(ast::hash const& hash)
        {
            values::hash new_hash;

            if (hash.elements()) {
                for (auto& element : *hash.elements()) {
                    new_hash.emplace(_evaluator.evaluate(element.first), _evaluator.evaluate(element.second));
                }
            }
            return new_hash;
        }

     private:
        expression_evaluator& _evaluator;
    };

    struct control_flow_expression_visitor : boost::static_visitor<value>
    {
        explicit control_flow_expression_visitor(expression_evaluator& evaluator) :
            _evaluator(evaluator)
        {
        }

        result_type operator()(ast::case_expression const& expr)
        {
            // Case expressions create a new match scope
            match_variable_scope match_scope(_evaluator.context().current());

            // Evaluate the case's expression
            value result = _evaluator.evaluate(expr.expression());

            auto& propositions = expr.propositions();
            boost::optional<size_t> default_index;
            for (size_t i = 0; i < propositions.size(); ++i) {
                auto& proposition = propositions[i];

                // Check for a lambda proposition
                if (proposition.lambda()) {
                    // Automatically splat an array
                    values::array arguments;
                    if (auto ptr = as<values::array>(result)) {
                        arguments = *ptr;
                    } else {
                        arguments.push_back(result);
                    }

                    // Yield to the lambda and execute the block if truthy
                    runtime::yielder yielder(_evaluator, proposition.position(), proposition.lambda());
                    if (is_truthy(yielder.yield(arguments))) {
                        return execute_block(proposition.body());
                    }
                    continue;
                }

                // Look for a match in the options
                for (auto& option : proposition.options()) {
                    // Evaluate the option
                    value option_value = _evaluator.evaluate(option);
                    if (is_default(option_value)) {
                        // Remember where the default is and keep going
                        default_index = i;
                        continue;
                    }

                    // If unfolding, treat each element as an option
                    auto unfold_array = _evaluator.unfold(option, option_value);
                    if (unfold_array) {
                        for (auto& element : *unfold_array) {
                            if (is_match(_evaluator.context(), result, expr.position(), element, option.position())) {
                                return execute_block(proposition.body());
                            }
                        }
                    }

                    if (is_match(_evaluator.context(), result, expr.position(), option_value, option.position())) {
                        return execute_block(proposition.body());
                    }
                }
            }

            // Handle no matching case
            if (default_index) {
                return execute_block(propositions[*default_index].body());
            }

            // Nothing matched, return undef
            return value();
        }

        result_type operator()(ast::if_expression const& expr)
        {
            // If expressions create a new match scope
            match_variable_scope match_scope(_evaluator.context().current());

            if (is_truthy(_evaluator.evaluate(expr.conditional()))) {
                return execute_block(expr.body());
            }
            if (expr.elsifs()) {
                for (auto& elsif : *expr.elsifs()) {
                    if (is_truthy(_evaluator.evaluate(elsif.conditional()))) {
                        return execute_block(elsif.body());
                    }
                }
            }
            if (expr.else_()) {
                return execute_block(expr.else_()->body());
            }
            return value();
        }

        result_type operator()(ast::unless_expression const& expr)
        {
            // Unless expressions create a new match scope
            match_variable_scope match_scope(_evaluator.context().current());

            if (!is_truthy(_evaluator.evaluate(expr.conditional()))) {
                return execute_block(expr.body());
            }
            if (expr.else_()) {
                return execute_block(expr.else_()->body());
            }
            return value();
        }

        result_type operator()(ast::function_call_expression const& expr)
        {
            runtime::dispatcher dispatcher(expr.function().value(), expr.position());
            return dispatcher.dispatch(_evaluator, expr.arguments(), expr.lambda());
        }

     private:
        result_type execute_block(boost::optional<vector<ast::expression>> const& expressions)
        {
            value result;
            if (expressions) {
                for (size_t i = 0; i < expressions->size(); ++i) {
                    auto& expression = (*expressions)[i];
                    // The last expression in the block is allowed to be unproductive (i.e. the return value)
                    result = _evaluator.evaluate(expression, i < (expressions->size() - 1));
                }
            }
            return result;
        }

        expression_evaluator& _evaluator;
    };

    struct access_expression_visitor : boost::static_visitor<value>
    {
        access_expression_visitor(expression_evaluator& evaluator, vector<ast::expression> const& expressions, token_position const& position) :
            _evaluator(evaluator),
            _position(position)
        {
            // Evaluate all the expressions for the access
            for (auto& expression : expressions) {
                auto argument = _evaluator.evaluate(expression);

                // If unfolding, append the array's elements
                auto unfold_array = _evaluator.unfold(expression, argument);
                if (unfold_array) {
                    _positions.insert(_positions.end(), unfold_array->size(), expression.position());
                    _arguments.reserve(_arguments.size() + unfold_array->size());
                    _arguments.insert(_arguments.end(), std::make_move_iterator(unfold_array->begin()), std::make_move_iterator(unfold_array->end()));
                    continue;
                }
                _positions.push_back(expression.position());
                _arguments.emplace_back(std::move(argument));
            }
        }

        result_type operator()(string const& target)
        {
            if (_arguments.size() > 2) {
                throw evaluation_exception(_positions[2], (boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::string::name() % _arguments.size()).str());
            }

            // Get the index
            auto ptr = as<int64_t>(_arguments[0]);
            if (!ptr) {
                throw evaluation_exception(_positions[0], (boost::format("expected %1% for start index but found %2%.") % types::integer::name() % get_type(_arguments[0])).str());
            }

            // If the index is negative, it's from the end of the string
            int64_t index = *ptr;
            if (index < 0) {
                index += static_cast<int64_t>(target.size());
            }

            // Get the count
            int64_t count = 1;
            if (_arguments.size() == 2) {
                ptr = as<int64_t>(_arguments[1]);
                if (!ptr) {
                    throw evaluation_exception(_positions[1], (boost::format("expected %1% for count but found %2%.") % types::integer::name() % get_type(_arguments[1])).str());
                }
                count = *ptr;

                // A negative count denotes an end index (inclusive)
                if (count < 0) {
                    count += (target.size() + 1 - index);
                }
            }

            // If the index is still to the "left" of the start of the string, adjust the count and start at index 0
            if (index < 0) {
                count += index;
                index = 0;
            }
            if (count <= 0) {
                return string();
            }
            return target.substr(static_cast<size_t>(index), static_cast<size_t>(count));
        }

        result_type operator()(values::array const& target)
        {
            if (_arguments.size() > 2) {
                throw evaluation_exception(_positions[2], (boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::array::name() % _arguments.size()).str());
            }

            // Get the index
            auto ptr = as<int64_t>(_arguments[0]);
            if (!ptr) {
                throw evaluation_exception(_positions[0], (boost::format("expected %1% for start index but found %2%.") % types::integer::name() % get_type(_arguments[0])).str());
            }

            // If the index is negative, it's from the end of the array
            int64_t index = *ptr;
            if (index < 0) {
                index += static_cast<int64_t>(target.size());
            }

            // Get the count
            int64_t count = 1;
            if (_arguments.size() == 2) {
                ptr = as<int64_t>(_arguments[1]);
                if (!ptr) {
                    throw evaluation_exception(_positions[1], (boost::format("expected %1% for count but found %2%.") % types::integer::name() % get_type(_arguments[1])).str());
                }
                count = *ptr;

                // A negative count denotes an end index (inclusive)
                if (count < 0) {
                    count += (target.size() + 1 - index);
                }
            } else {
                // Only the index given; return the element itself if in range
                if (index < 0 || static_cast<size_t>(index) >= target.size()) {
                    return value();
                }
                return target[index];
            }

            // If the index is still to the "left" of the start of the array, adjust the count and start at index 0
            if (index < 0) {
                count += index;
                index = 0;
            }
            if (count <= 0) {
                return values::array();
            }

            // Create the subarray
            values::array subarray;
            subarray.reserve(count);
            for (int64_t i = 0; i < count && static_cast<size_t>(i + index) < target.size(); ++i) {
                subarray.emplace_back(target[i + index]);
            }
            return subarray;
        }

        result_type operator()(values::hash const& target)
        {
            if (_arguments.size() == 1) {
                // Lookup by key
                auto it = target.find(_arguments[0]);
                if (it == target.end()) {
                    return value();
                }
                return it->second;
            }

            // Otherwise, build an array of values
            values::array result;
            for (auto const& argument : _arguments) {
                // Lookup by key
                auto it = target.find(argument);
                if (it == target.end()) {
                    continue;
                }
                result.emplace_back(it->second);
            }
            return result;
        }

        result_type operator()(type const& target)
        {
            return boost::apply_visitor(*this, target);
        }

        result_type operator()(types::integer const& type)
        {
            // At most 2 arguments to Integer
            if (_arguments.size() > 2) {
                throw evaluation_exception(_positions[2], (boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::integer::name() % _arguments.size()).str());
            }

            int64_t from, to;
            tie(from, to) = get_range<int64_t, types::integer>();
            return types::integer(from, to);
        }

        result_type operator()(types::floating const& type)
        {
            // At most 2 arguments to Float
            if (_arguments.size() > 2) {
                throw evaluation_exception(_positions[2], (boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::floating::name() % _arguments.size()).str());
            }

            long double from, to;
            tie(from, to) = get_range<long double, types::floating>();
            return types::floating(from, to);
        }

        result_type operator()(types::string const& type)
        {
            // At most 2 arguments to String
            if (_arguments.size() > 2) {
                throw evaluation_exception(_positions[2], (boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::string::name() % _arguments.size()).str());
            }

            int64_t from, to;
            tie(from, to) = get_range<int64_t, types::integer>(true);
            return types::string(from, to);
        }

        result_type operator()(types::regexp const& type)
        {
            // At most 1 arguments to Regexp
            if (_arguments.size() > 1) {
                throw evaluation_exception(_positions[1], (boost::format("expected at most 1 arguments for %1% but %2% were given.") % types::regexp::name() % _arguments.size()).str());
            }

            // Get the pattern argument; check for regex argument first
            string pattern;
            auto regex = as<values::regex>(_arguments[0]);
            if (regex) {
                pattern = regex->pattern();
            } else {
                if (!as<string>(_arguments[0])) {
                    throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% or %2% but found %3%.") % types::string::name() % types::regexp::name() % get_type(_arguments[0])).str());
                }
                pattern = mutate_as<string>(_arguments[0]);
            }
            return types::regexp(std::move(pattern));
        }

        result_type operator()(types::enumeration const& type)
        {
            // Ensure each argument is a string
            vector<string> strings;
            strings.reserve(_arguments.size());

            for (size_t i = 0; i < _arguments.size(); ++i) {
                if (!as<string>(_arguments[i])) {
                    throw evaluation_exception(_positions[i], (boost::format("expected %1% but found %2%.") % types::string::name() % get_type(_arguments[i])).str());
                }
                strings.emplace_back(mutate_as<string>(_arguments[i]));
            }
            return types::enumeration(std::move(strings));
        }

        result_type operator()(types::pattern const& type)
        {
            vector<values::regex> patterns;
            patterns.reserve(_arguments.size());

            // Each argument can be a string, regex value, Regexp type or another Pattern type
            for (size_t i = 0; i < _arguments.size(); ++i) {
                // Check for string
                if (as<string>(_arguments[i])) {
                    patterns.emplace_back(mutate_as<string>(_arguments[i]));
                    continue;
                }
                // Check for regex
                if (as<values::regex>(_arguments[i])) {
                    patterns.emplace_back(mutate_as<values::regex>(_arguments[i]));
                    continue;
                }
                // Check for Type
                auto type_ptr = as<values::type>(_arguments[i]);
                if (type_ptr) {
                    auto regexp = boost::get<types::regexp>(type_ptr);
                    if (regexp) {
                        patterns.emplace_back(regexp->pattern());
                        continue;
                    }
                    // Check for Pattern type
                    auto pattern_type = boost::get<types::pattern>(type_ptr);
                    if (pattern_type) {
                        patterns.reserve(patterns.size() + pattern_type->patterns().size());
                        patterns.insert(patterns.end(), pattern_type->patterns().begin(), pattern_type->patterns().end());
                        continue;
                    }
                }

                // Not any of the above types
                throw evaluation_exception(_positions[i], (boost::format("expected %1%, %2%, or %3% but found %4%.") %
                        types::string::name() %
                        types::regexp::name() %
                        types::pattern::name() %
                        get_type(_arguments[i])).str());
            }
            return types::pattern(std::move(patterns));
        }

        result_type operator()(types::array const& type)
        {
            // At most 3 arguments to Array
            if (_arguments.size() > 3) {
                throw evaluation_exception(_positions[3], (boost::format("expected at most 3 arguments for %1% but %2% were given.") % types::array::name() % _arguments.size()).str());
            }

            // First argument should be a type
            if (!as<values::type>(_arguments[0])) {
                throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[0])).str());
            }

            // Get the optional range
            size_t from, to;
            tie(from, to) = get_range<int64_t, types::integer>(true, 1);
            return types::array(mutate_as<values::type>(_arguments[0]), from, to);
        }

        result_type operator()(types::hash const& type)
        {
            // At least 2 and at most 4 arguments to Hash
            if (_arguments.size() < 2) {
                throw evaluation_exception(_position, (boost::format("expected at least 2 arguments for %1% but %2% were given.") % types::hash::name() % _arguments.size()).str());
            }
            if (_arguments.size() > 4) {
                throw evaluation_exception(_positions[4], (boost::format("expected at most 3 arguments for %1% but %2% were given.") % types::hash::name() % _arguments.size()).str());
            }

            // First argument should be a type
            if (!as<values::type>(_arguments[0])) {
                throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[0])).str());
            }

            // Second argument should be a type
            if (!as<values::type>(_arguments[1])) {
                throw evaluation_exception(_positions[1], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[1])).str());
            }

            // Get the optional range
            size_t from, to;
            tie(from, to) = get_range<int64_t, types::integer>(true, 2);
            return types::hash(mutate_as<values::type>(_arguments[0]), mutate_as<values::type>(_arguments[1]), from, to);
        }

        result_type operator()(types::tuple const& type)
        {
            vector<values::type> types;
            types.reserve(_arguments.size());

            int64_t from = _arguments.size();
            int64_t to = _arguments.size();
            for (size_t i = 0; i < _arguments.size(); ++i) {
                // Stop at first parameter that isn't a type
                if (!as<values::type>(_arguments[i])) {
                    // There must be at most 2 more parameters (the range)
                    if ((i + 2) < _arguments.size()) {
                        throw evaluation_exception(_positions[i + 2], (boost::format("expected at most %1% arguments for %2% but %3% were given.") % (i + 2) % types::tuple::name() % _arguments.size()).str());
                    }
                    // Get the optional range
                    tie(from, to) = get_range<int64_t, types::integer>(true, i);
                    break;
                }
                types.emplace_back(mutate_as<values::type>(_arguments[i]));
            }
            return types::tuple(std::move(types), from, to);
        }

        result_type operator()(types::optional const& type)
        {
            // Only 1 argument to Optional
            if (_arguments.size() > 1) {
                throw evaluation_exception(_positions[2], (boost::format("expected 1 argument for %1% but %2% were given.") % types::optional::name() % _arguments.size()).str());
            }

            // First argument should be a type
            if (!as<values::type>(_arguments[0])) {
                throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[0])).str());
            }

            return types::optional(mutate_as<values::type>(_arguments[0]));
        }

        result_type operator()(types::type const& type)
        {
            // Only 1 argument to Type
            if (_arguments.size() > 1) {
                throw evaluation_exception(_positions[2], (boost::format("expected 1 argument for %1% but %2% were given.") % types::type::name() % _arguments.size()).str());
            }

            // First argument should be a type
            if (!as<values::type>(_arguments[0])) {
                throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[0])).str());
            }

            return types::type(mutate_as<values::type>(_arguments[0]));
        }

        result_type operator()(types::structure const& type)
        {
            // Only 1 argument to Struct
            if (_arguments.size() > 1) {
                throw evaluation_exception(_positions[2], (boost::format("expected 1 argument for %1% but %2% were given.") % types::structure::name() % _arguments.size()).str());
            }

            // First argument should be a hash
            if (!as<values::hash>(_arguments[0])) {
                throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::hash::name() % get_type(_arguments[0])).str());
            }

            auto hash = mutate_as<values::hash>(_arguments[0]);

            // Build a map of string -> Type
            unordered_map<string, values::type> types;
            for (auto& kvp : hash) {
                // Ensure the key is a string
                auto key = as<string>(kvp.first);
                if (!key) {
                    throw evaluation_exception(_positions[0], (boost::format("expected hash keys to be %1% but found %2%.") % types::string::name() % get_type(kvp.first)).str());
                }
                // Ensure the value is a type
                if (!as<values::type>(kvp.second)) {
                    throw evaluation_exception(_positions[0], (boost::format("expected hash values to be %1% but found %2%.") % types::type::name() % get_type(kvp.second)).str());
                }
                types.insert(make_pair(*key, mutate_as<values::type>(kvp.second)));
            }
            return types::structure(std::move(types));
        }

        result_type operator()(types::variant const& type)
        {
            vector<values::type> types;
            types.reserve(_arguments.size());

            for (size_t i = 0; i < _arguments.size(); ++i) {
                if (!as<values::type>(_arguments[i])) {
                    throw evaluation_exception(_positions[i], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[i])).str());
                }
                types.emplace_back(mutate_as<values::type>(_arguments[i]));
            }
            return types::variant(std::move(types));
        }

        result_type operator()(types::resource const& type)
        {
            // If the resource already has a type, treat the access as titles only
            if (!type.type_name().empty()) {
                // If there is only one parameter, return a resource with a title
                if (_arguments.size() == 1) {
                    if (!as<string>(_arguments[0])) {
                        throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::string::name() % get_type(_arguments[0])).str());
                    }
                    return types::resource(type.type_name(), mutate_as<string>(_arguments[0]));
                }

                // Otherwise, return an array of resources with titles
                values::array result;
                for (size_t i = 0; i < _arguments.size(); ++i) {
                    if (!as<string>(_arguments[i])) {
                        throw evaluation_exception(_positions[i], (boost::format("expected parameter to be %1% but found %2%.") % types::string::name() % get_type(_arguments[i])).str());
                    }
                    result.emplace_back(types::resource(type.type_name(), mutate_as<string>(_arguments[i])));
                }
                return result;
            }

            // Get the type name
            string type_name;
            if (as<string>(_arguments[0])) {
                type_name = mutate_as<string>(_arguments[0]);
            } else {
                auto type_ptr = as<values::type>(_arguments[0]);
                if (type_ptr) {
                    auto resource_ptr = boost::get<types::resource>(type_ptr);
                    if (resource_ptr) {
                        type_name = resource_ptr->type_name();
                    }
                }
            }
            if (type_name.empty()) {
                throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% or typed %2% but found %3%.") % types::string::name() % types::resource::name() % get_type(_arguments[0])).str());
            }

            // If only one parameter, return a resource of that type
            if (_arguments.size() == 1) {
                return types::resource(std::move(type_name));
            }

            // If there are two parameters, return a type with a title
            if (_arguments.size() == 2) {
                if (!as<string>(_arguments[1])) {
                    throw evaluation_exception(_positions[1], (boost::format("expected parameter to be %1% but found %2%.") % types::string::name() % get_type(_arguments[1])).str());
                }
                return types::resource(std::move(type_name), mutate_as<string>(_arguments[1]));
            }

            // Otherwise, return an array of types
            values::array result;
            for (size_t i = 1; i < _arguments.size(); ++i) {
                if (!as<string>(_arguments[i])) {
                    throw evaluation_exception(_positions[i], (boost::format("expected parameter to be %1% but found %2%.") % types::string::name() % get_type(_arguments[i])).str());
                }
                result.emplace_back(types::resource(type_name, mutate_as<string>(_arguments[i])));
            }
            return result;
        }

        template <typename T>
        result_type operator()(T const& target)
        {
            throw evaluation_exception(_position, (boost::format("access expression is not supported for %1%.") % get_type(target)).str());
        }

     private:
        template <typename Value, typename Type>
        tuple<Value, Value> get_range(bool accept_range = false, size_t start_index = 0) const
        {
            // Check for Integer type first
            if (accept_range && _arguments.size() > start_index) {
                auto type_ptr = as<values::type>(_arguments[start_index]);
                if (type_ptr) {
                    auto integer_ptr = boost::get<types::integer>(type_ptr);
                    if (integer_ptr) {
                        return make_tuple(integer_ptr->from(), integer_ptr->to());
                    }
                }
            }

            // Get the from argument
            Value from = numeric_limits<Value>::min();
            if (_arguments.size() > start_index) {
                auto& argument = _arguments[start_index];

                if (!is_default(argument)) {
                    // Try int64_t first; this allows either Integer or Floats when Value is floating point
                    auto ptr = as<int64_t>(argument);
                    if (ptr) {
                        from = static_cast<Value>(*ptr);
                    } else {
                        auto value_ptr = as<Value>(argument);
                        if (!value_ptr) {
                            throw evaluation_exception(_positions[start_index], (boost::format("expected parameter to be %1% but found %2%.") % Type::name() % get_type(argument)).str());
                        }
                        from = *value_ptr;
                    }
                }
            }
            // Get the to argument
            Value to = numeric_limits<Value>::max();
            if (_arguments.size() > (start_index + 1)) {
                auto& argument = _arguments[start_index + 1];

                if (!is_default(argument)) {
                    // Try int64_t first; this allows either Integer or Floats when Value is floating point
                    auto ptr = as<int64_t>(argument);
                    if (ptr) {
                        to = static_cast<Value>(*ptr);
                    } else {
                        auto value_ptr = as<Value>(argument);
                        if (!value_ptr) {
                            throw evaluation_exception(_positions[start_index], (boost::format("expected parameter to be %1% but found %2%.") % Type::name() % get_type(argument)).str());
                        }
                        to = *value_ptr;
                    }
                }
            }
            return make_tuple(from, to);
        }

        expression_evaluator& _evaluator;
        values::array _arguments;
        vector<lexer::token_position> _positions;
        token_position const& _position;
    };

    struct postfix_expression_visitor : boost::static_visitor<void>
    {
        postfix_expression_visitor(expression_evaluator& evaluator, value& result, ast::primary_expression const* first_expression, token_position& position) :
            _evaluator(evaluator),
            _result(result),
            _first_expression(first_expression),
            _position(position)
        {
        }

        result_type operator()(ast::selector_expression const& expr)
        {
            // Selector expressions create a new match scope
            match_variable_scope match_scope(_evaluator.context().current());

            boost::optional<size_t> default_index;

            auto& cases = expr.cases();
            for (size_t i = 0; i < cases.size(); ++i) {
                auto& selector_case = cases[i];

                // Evaluate the option
                value selector = _evaluator.evaluate(selector_case.selector());
                if (is_default(selector)) {
                    // Remember where the default case is and keep going
                    default_index = i;
                    continue;
                }

                // If unfolding, treat each element as an option
                auto unfold_array = _evaluator.unfold(selector_case.selector(), selector);
                if (unfold_array) {
                    for (auto& element : *unfold_array) {
                        if (is_match(_evaluator.context(), _result, expr.position(), element, selector_case.position())) {
                            _result = _evaluator.evaluate(selector_case.result());
                            _position = selector_case.position();
                            _first_expression = nullptr;
                            return;
                        }
                    }
                }

                if (is_match(_evaluator.context(), _result, expr.position(), selector, selector_case.position())) {
                    _result = _evaluator.evaluate(selector_case.result());
                    _position = selector_case.position();
                    _first_expression = nullptr;
                    return;
                }
            }

            // Handle no matching case
            if (!default_index) {
                throw evaluation_exception(expr.position(), (boost::format("no matching selector case for value '%1%'.") % _result).str());
            }

            // Evaluate the default case
            _result = _evaluator.evaluate(cases[*default_index].result());
            _position = cases[*default_index].position();
            _first_expression = nullptr;
        }

        result_type operator()(ast::access_expression const& expr)
        {
            access_expression_visitor visitor(_evaluator, expr.arguments(), expr.position());
            _result = boost::apply_visitor(visitor, dereference(_result));
            _position = expr.position();
            _first_expression = nullptr;
        }

        result_type operator()(ast::method_call_expression const& expr)
        {
            runtime::dispatcher dispatcher(expr.method().value(), expr.method().position());

            // Evaluate the result for the next call
            _result = dispatcher.dispatch(_evaluator, expr.arguments(), expr.lambda(), &_result, _first_expression, &_position);
            _position = expr.position();
            _first_expression = nullptr;
        }

    private:
        expression_evaluator& _evaluator;
        value& _result;
        ast::primary_expression const* _first_expression;
        token_position& _position;
    };

    struct primary_expression_visitor : boost::static_visitor<value>
    {
        explicit primary_expression_visitor(expression_evaluator& evaluator) :
            _evaluator(evaluator)
        {
        }

        result_type operator()(boost::blank const&)
        {
            return value();
        }

        result_type operator()(ast::basic_expression const& expr)
        {
            basic_expression_visitor visitor(_evaluator);
            return boost::apply_visitor(visitor, expr);
        }

        result_type operator()(ast::control_flow_expression const& expr)
        {
            control_flow_expression_visitor visitor(_evaluator);
            return boost::apply_visitor(visitor, expr);
        }

        result_type operator()(ast::catalog_expression const& expr)
        {
            // TODO: implement
            throw evaluation_exception(get_position(expr), "catalog expression not yet implemented.");
        }

        result_type operator()(ast::unary_expression const& expr)
        {
            static const unordered_map<ast::unary_operator, std::function<values::value(operators::unary_context&)>> unary_operators = {
                { ast::unary_operator::negate,      operators::negate() },
                { ast::unary_operator::logical_not, operators::logical_not() },
                { ast::unary_operator::splat,       operators::splat() }
            };

            auto it = unary_operators.find(expr.op());
            if (it == unary_operators.end()) {
                throw evaluation_exception(expr.position(), "unexpected unary expression.");
            }

            auto operand = boost::apply_visitor(*this, expr.operand());
            operators::unary_context context(_evaluator.context(), operand, expr.position());
            return it->second(context);
        }

        result_type operator()(ast::postfix_expression const& expr)
        {
            auto result = boost::apply_visitor(*this, expr.primary());
            auto position = expr.position();

            // Evaluate the postfix expressions
            postfix_expression_visitor visitor(_evaluator, result, &expr.primary(), position);
            for (auto const& expression : expr.subexpressions()) {
                boost::apply_visitor(visitor, expression);
            }
            return result;
        }

        result_type operator()(ast::expression const& expr)
        {
            return _evaluator.evaluate(expr);
        }

     private:
        expression_evaluator& _evaluator;
    };

    expression_evaluator::expression_evaluator(runtime::context& ctx) :
        _context(ctx)
    {
    }

    value expression_evaluator::evaluate(ast::expression const& expr, bool productive)
    {
        if (productive && !is_productive(expr)) {
            throw evaluation_exception(expr.position(), "unproductive expressions may only appear last in a block.");
        }

        // Evaluate the primary expression
        auto result = evaluate(expr.primary());
        auto position = expr.position();

        // Climb the remainder of the expression
        auto& binary = expr.binary();
        auto begin = binary.begin();
        climb_expression(result, position, 0, begin, binary.end());

        return result;
    }

    value expression_evaluator::evaluate(ast::primary_expression const& expr)
    {
        primary_expression_visitor visitor(*this);
        return boost::apply_visitor(visitor, expr);
    }

    runtime::context const& expression_evaluator::context() const
    {
        return _context;
    }

    runtime::context& expression_evaluator::context()
    {
        return _context;
    }

    boost::optional<values::array> expression_evaluator::unfold(ast::expression const& expr, value& result)
    {
        // An unfold expression is always unary with no further expressions
        if (!expr.binary().empty()) {
            return boost::none;
        }
        // Unfold the first expression
        return unfold(expr.primary(), result);
    }

    boost::optional<values::array> expression_evaluator::unfold(ast::primary_expression const& expression, value& evaluated)
    {
        // Determine if the given expression is a unary splat of an array
        auto unary = boost::get<ast::unary_expression>(&expression);
        if (unary && unary->op() == ast::unary_operator::splat && as<values::array>(evaluated)) {
            return mutate_as<values::array>(evaluated);
        }

        // Try for nested expression
        auto nested = boost::get<ast::expression>(&expression);
        if (nested) {
            return unfold(*nested, evaluated);
        }
        return boost::none;
    }

    bool expression_evaluator::is_productive(ast::expression const& expr)
    {
        // Check if the primary expression itself is productive
        if (is_productive(expr.primary())) {
            return true;
        }

        // Expressions followed by an assignment or relationship operator are productive
        for (auto const& binary : expr.binary()) {
            if (binary.op() == ast::binary_operator::assignment ||
                binary.op() == ast::binary_operator::in_edge ||
                binary.op() == ast::binary_operator::in_edge_subscribe ||
                binary.op() == ast::binary_operator::out_edge ||
                binary.op() == ast::binary_operator::out_edge_subscribe) {
                return true;
            }
        }

        return false;
    }

    bool expression_evaluator::is_productive(ast::primary_expression const& expr)
    {
        // Check for recursive primary expression
        if (auto ptr = boost::get<ast::expression>(&expr)) {
            if (is_productive(*ptr)) {
                return true;
            }
        }

        // Check for unary expression
        if (auto ptr = boost::get<ast::unary_expression>(&expr)) {
            if (is_productive(ptr->operand())) {
                return true;
            }
        }

        // Catalog and control flow expressions are productive
        if (boost::get<ast::catalog_expression>(&expr) ||
            boost::get<ast::control_flow_expression>(&expr)) {
            return true;
        }

        // Postfix method calls are productive
        if (auto ptr = boost::get<ast::postfix_expression>(&expr)) {
            if (is_productive(ptr->primary())) {
                return true;
            }
            for (auto const& subexpression : ptr->subexpressions()) {
                if (boost::get<ast::method_call_expression>(&subexpression)) {
                    return true;
                }
            }
        }
        return false;
    }

    void expression_evaluator::climb_expression(
        value& left,
        token_position& left_position,
        uint8_t min_precedence,
        vector<ast::binary_expression>::const_iterator& begin,
        vector<ast::binary_expression>::const_iterator const& end)
    {
        // This member implements precedence climbing for binary expressions
        uint8_t precedence;
        while (begin != end && (precedence = get_precedence(begin->op())) >= min_precedence)
        {
            auto op = begin->op();
            auto& operand = begin->operand();
            auto right_position = begin->position();
            ++begin;

            // Evaluate the right side
            value right = evaluate(operand);

            // Recurse and climb the expression
            uint8_t next_precdence = precedence + (is_right_associative(op) ? static_cast<uint8_t>(0) : static_cast<uint8_t>(1));
            climb_expression(right, right_position, next_precdence, begin, end);

            // Evaluate this part of the expression
            evaluate(left, left_position, op, right, right_position);
        }
    }

    void expression_evaluator::evaluate(
        value& left,
        token_position const& left_position,
        ast::binary_operator op,
        value& right,
        token_position& right_position)
    {
        static const unordered_map<ast::binary_operator, std::function<values::value(operators::binary_context&)>> binary_operators = {
            { ast::binary_operator::assignment,         operators::assignment() },
            { ast::binary_operator::divide,             operators::divide() },
            { ast::binary_operator::equals,             operators::equals() },
            { ast::binary_operator::greater_than,       operators::greater() },
            { ast::binary_operator::greater_equals,     operators::greater_equal() },
            { ast::binary_operator::in,                 operators::in() },
            { ast::binary_operator::less_than,          operators::less() },
            { ast::binary_operator::less_equals,        operators::less_equal() },
            { ast::binary_operator::left_shift,         operators::left_shift() },
            { ast::binary_operator::logical_and,        operators::logical_and() },
            { ast::binary_operator::logical_or,         operators::logical_or() },
            { ast::binary_operator::match,              operators::match() },
            { ast::binary_operator::minus,              operators::minus() },
            { ast::binary_operator::modulo,             operators::modulo() },
            { ast::binary_operator::multiply,           operators::multiply() },
            { ast::binary_operator::not_equals,         operators::not_equals() },
            { ast::binary_operator::not_match,          operators::not_match() },
            { ast::binary_operator::plus,               operators::plus() },
            { ast::binary_operator::right_shift,        operators::right_shift() }
        };

        auto it = binary_operators.find(op);
        if (it == binary_operators.end()) {
            throw evaluation_exception(left_position, "unexpected binary expression.");
        }

        operators::binary_context context(_context, left, left_position, right, right_position);
        left = it->second(context);
    }

    uint8_t expression_evaluator::get_precedence(ast::binary_operator op)
    {
        // Return the precedence (low to high)
        switch (op) {
            case ast::binary_operator::in_edge:
            case ast::binary_operator::in_edge_subscribe:
            case ast::binary_operator::out_edge:
            case ast::binary_operator::out_edge_subscribe:
                return 1;

            case ast::binary_operator::assignment:
                return 2;

            case ast::binary_operator::logical_or:
                return 3;

            case ast::binary_operator::logical_and:
                return 4;

            case ast::binary_operator::greater_than:
            case ast::binary_operator::greater_equals:
            case ast::binary_operator::less_than:
            case ast::binary_operator::less_equals:
                return 5;

            case ast::binary_operator::equals:
            case ast::binary_operator::not_equals:
                return 6;

            case ast::binary_operator::left_shift:
            case ast::binary_operator::right_shift:
                return 7;

            case ast::binary_operator::plus:
            case ast::binary_operator::minus:
                return 8;

            case ast::binary_operator::multiply:
            case ast::binary_operator::divide:
            case ast::binary_operator::modulo:
                return 9;

            case ast::binary_operator::match:
            case ast::binary_operator::not_match:
                return 10;

            case ast::binary_operator::in:
                return 11;

            default:
                break;
        }

        throw runtime_error("invalid binary operator.");
    }

    bool expression_evaluator::is_right_associative(ast::binary_operator op)
    {
        return op == ast::binary_operator::assignment;
    }

}}  // namespace puppet::runtime
