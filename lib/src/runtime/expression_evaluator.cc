#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/runtime/operators.hpp>
#include <puppet/runtime/string_interpolator.hpp>
#include <puppet/runtime/dispatcher.hpp>
#include <puppet/ast/expression_def.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

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

        result_type operator()(ast::undef const&) const
        {
            return value();
        }

        result_type operator()(ast::defaulted const&) const
        {
            return defaulted();
        }

        result_type operator()(ast::boolean const& boolean) const
        {
            return boolean.value();
        }

        result_type operator()(int64_t integer) const
        {
            return integer;
        }

        result_type operator()(long double floating) const
        {
            return floating;
        }

        result_type operator()(ast::number const& number) const
        {
            return boost::apply_visitor(*this, number.value());
        }

        result_type operator()(ast::string const& str) const
        {
            string_interpolator interpolator(_evaluator);
            return interpolator.interpolate(str.position(), str.value(), str.escapes(), str.quote(), str.interpolated(), str.margin(), str.remove_break());
        }

        result_type operator()(ast::regex const& regx) const
        {
            try {
                return values::regex(regx.value());
            } catch (std::regex_error const& ex) {
                throw evaluation_exception(regx.position(), ex.what());
            }
        }

        result_type operator()(ast::variable const& var) const
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

        result_type operator()(ast::name const& name) const
        {
            // Treat as a string
            return name.value();
        }

        result_type operator()(ast::bare_word const& word) const
        {
            // Treat as a string
            return word.value();
        }

        result_type operator()(ast::type const& type) const
        {
            static const unordered_map<string, values::type> names = {
                { types::any::name(),           types::any() },
                { types::array::name(),         types::array() },
                { types::boolean::name(),       types::boolean() },
                { types::callable::name(),      types::callable() },
                { types::data::name(),          types::data() },
                { types::defaulted::name(),     types::defaulted() },
                { types::floating::name(),      types::floating() },
                { types::hash::name(),          types::hash() },
                { types::integer::name(),       types::integer() },
                { types::numeric::name(),       types::numeric() },
                { types::regexp::name(),        types::regexp() },
                { types::scalar::name(),        types::scalar() },
                { types::string::name(),        types::string() },
                { types::structure::name(),     types::structure() },
                { types::tuple::name(),         types::tuple() },
                { types::type::name(),          types::type(boost::none) },
                { types::undef::name(),         types::undef() }
            };

            auto it = names.find(type.name());
            if (it == names.end()) {
                throw evaluation_exception(type.position(), (boost::format("unknown type %1%.") % type.name()).str());
            }
            return it->second;
        }

        result_type operator()(ast::array const& arr) const
        {
            values::array new_array;

            if (arr.elements()) {
                for (auto& element : *arr.elements()) {
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

        result_type operator()(ast::hash const& h) const
        {
            values::hash new_hash;

            if (h.elements()) {
                for (auto& element : *h.elements()) {
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

        result_type operator()(ast::selector_expression const& expr) const
        {
            // Selector expressions create a new match scope
            match_variable_scope match_scope(_evaluator.context().current());

            // Evaluate the selector's value
            value result = _evaluator.evaluate(expr.value());

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
                        if (is_match(result, element, expr.position(), selector_case.position())) {
                            return _evaluator.evaluate(selector_case.result());
                        }
                    }
                }

                if (is_match(result, selector, expr.position(), selector_case.position())) {
                    return _evaluator.evaluate(selector_case.result());
                }
            }

            // Handle no matching case
            if (!default_index) {
                throw evaluation_exception(expr.position(), (boost::format("no matching selector case for value '%1%'.") % result).str());
            }

            // Evaluate the default case
            return _evaluator.evaluate(cases[*default_index].result());
        }

        result_type operator()(ast::case_expression const& expr) const
        {
            // Case expressions create a new match scope
            match_variable_scope match_scope(_evaluator.context().current());

            // Evaluate the case's expression
            value result = _evaluator.evaluate(expr.expression());

            boost::optional<size_t> default_index;

            auto& propositions = expr.propositions();
            for (size_t i = 0; i < propositions.size(); ++i) {
                auto& proposition = propositions[i];
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
                            if (is_match(result, element, expr.position(), option.position())) {
                                return execute_block(proposition.body());
                            }
                        }
                    }

                    if (is_match(result, option_value, expr.position(), option.position())) {
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

        result_type operator()(ast::if_expression const& expr) const
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

        result_type operator()(ast::unless_expression const& expr) const
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

        result_type operator()(ast::method_call_expression const& expr) const
        {
            // Evaluate the target first and get the position
            value result = _evaluator.evaluate(expr.target());
            auto position = &ast::get_position(expr.target());

            for (auto const& call : expr.calls()) {
                runtime::dispatcher dispatcher(call.method().value(), call.position());

                // Evaluate the result for the next call
                result = dispatcher.dispatch(_evaluator, call.arguments(), call.lambda(), &result, position);

                // Subsequent calls use the call position for the first argument
                position = nullptr;
            }
            return result;
        }

        result_type operator()(ast::function_call_expression const& expr) const
        {
            runtime::dispatcher dispatcher(expr.function().value(), expr.position());
            return dispatcher.dispatch(_evaluator, expr.arguments(), expr.lambda());
        }

     private:
        result_type execute_block(boost::optional<vector<ast::expression>> const& expressions) const
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

        bool is_match(value const& result, value const& expected, token_position const& result_position, token_position const& expected_position) const
        {
            // If the expected value is a regex, use match
            auto regex = boost::get<values::regex>(&dereference(expected));
            if (regex) {
                // Only match against strings
                if (boost::get<string>(&dereference(result))) {
                    if (is_truthy(match(result, expected, result_position, expected_position, _evaluator.context()))) {
                        return true;
                    }
                }
                return false;
            }

            // Otherwise, use equals
            return equals(result, expected);
        }

        expression_evaluator& _evaluator;
    };

    struct access_expression_evaluator : boost::static_visitor<value>
    {
        access_expression_evaluator(expression_evaluator& evaluator, vector<ast::expression> const& expressions, token_position const& position) :
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
            auto ptr = boost::get<int64_t>(&dereference(_arguments[0]));
            if (!ptr) {
                throw evaluation_exception(_positions[0], (boost::format("expected %1% for start index but found %2%.") % types::integer::name() % get_type_name(_arguments[0])).str());
            }

            // If the index is negative, it's from the end of the string
            int64_t index = *ptr;
            if (index < 0) {
                index += static_cast<int64_t>(target.size());
            }

            // Get the count
            int64_t count = 1;
            if (_arguments.size() == 2) {
                ptr = boost::get<int64_t>(&dereference(_arguments[1]));
                if (!ptr) {
                    throw evaluation_exception(_positions[1], (boost::format("expected %1% for count but found %2%.") % types::integer::name() % get_type_name(_arguments[1])).str());
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
            auto ptr = boost::get<int64_t>(&dereference(_arguments[0]));
            if (!ptr) {
                throw evaluation_exception(_positions[0], (boost::format("expected %1% for start index but found %2%.") % types::integer::name() % get_type_name(_arguments[0])).str());
            }

            // If the index is negative, it's from the end of the array
            int64_t index = *ptr;
            if (index < 0) {
                index += static_cast<int64_t>(target.size());
            }

            // Get the count
            int64_t count = 1;
            if (_arguments.size() == 2) {
                ptr = boost::get<int64_t>(&dereference(_arguments[1]));
                if (!ptr) {
                    throw evaluation_exception(_positions[1], (boost::format("expected %1% for count but found %2%.") % types::integer::name() % get_type_name(_arguments[1])).str());
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
                auto it = target.find(dereference(_arguments[0]));
                if (it == target.end()) {
                    return value();
                }
                return it->second;
            }

            // Otherwise, build an array of values
            values::array result;
            for (auto const& argument : _arguments) {
                // Lookup by key
                auto it = target.find(dereference(argument));
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
            auto regex = boost::get<values::regex>(&dereference(_arguments[0]));
            if (regex) {
                pattern = regex->pattern();
            } else {
                auto str = move_parameter<string>(0);
                if (!str) {
                    throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% or %2% but found %3%.") % types::string::name() % types::regexp::name() % get_type_name(_arguments[0])).str());
                }
                pattern = std::move(*str);
            }
            return types::regexp(std::move(pattern));
        }

        result_type operator()(types::array const& type)
        {
            // At most 3 arguments to Array
            if (_arguments.size() > 3) {
                throw evaluation_exception(_positions[3], (boost::format("expected at most 3 arguments for %1% but %2% were given.") % types::array::name() % _arguments.size()).str());
            }

            // First argument should be a type
            auto element_type = move_parameter<values::type>(0);
            if (!element_type) {
                throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type_name(_arguments[0])).str());
            }

            // Get the optional range
            size_t from, to;
            tie(from, to) = get_range<int64_t, types::integer>(true, 1);
            return types::array(std::move(*element_type), from, to);
        }

        result_type operator()(types::hash const& type)
        {
            // At least 2 and at most 4 arguments to Hash
            if (_arguments.size() < 2) {
                throw evaluation_exception(_positions[4], (boost::format("expected at least 2 arguments for %1% but %2% were given.") % types::hash::name() % _arguments.size()).str());
            }
            if (_arguments.size() > 4) {
                throw evaluation_exception(_positions[4], (boost::format("expected at most 3 arguments for %1% but %2% were given.") % types::hash::name() % _arguments.size()).str());
            }

            // First argument should be a type
            auto key_type = move_parameter<values::type>(0);
            if (!key_type) {
                throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type_name(_arguments[0])).str());
            }

            // Second argument should be a type
            auto element_type = move_parameter<values::type>(1);
            if (!element_type) {
                throw evaluation_exception(_positions[1], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type_name(_arguments[1])).str());
            }

            // Get the optional range
            size_t from, to;
            tie(from, to) = get_range<int64_t, types::integer>(true, 2);
            return types::hash(std::move(*key_type), std::move(*element_type), from, to);
        }

        result_type operator()(types::tuple const& type)
        {
            vector<values::type> types;
            types.reserve(_arguments.size());

            int64_t from = _arguments.size();
            int64_t to = _arguments.size();
            for (size_t i = 0; i < _arguments.size(); ++i) {
                auto type_parameter = move_parameter<values::type>(i);
                // Stop at first parameter that isn't a type
                if (!type_parameter) {
                    // There must be at most 2 more parameters (the range)
                    if ((i + 2) < _arguments.size()) {
                        throw evaluation_exception(_positions[i + 2], (boost::format("expected at most %1% arguments for %2% but %3% were given.") % (i + 2) % types::tuple::name() % _arguments.size()).str());
                    }
                    // Get the optional range
                    tie(from, to) = get_range<int64_t, types::integer>(true, i);
                    break;
                }
                types.emplace_back(std::move(*type_parameter));
            }
            return types::tuple(std::move(types), from, to);
        }

        result_type operator()(types::type const& type)
        {
            // Only 1 argument to Type
            if (_arguments.size() > 1) {
                throw evaluation_exception(_positions[2], (boost::format("expected 1 argument for %1% but %2% were given.") % types::type::name() % _arguments.size()).str());
            }

            // First argument should be a type
            auto underlying_type = move_parameter<values::type>(0);
            if (!underlying_type) {
                throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type_name(_arguments[0])).str());
            }

            return types::type(std::move(*underlying_type));
        }

        result_type operator()(types::structure const& type)
        {
            // Only 1 argument to Struct
            if (_arguments.size() > 1) {
                throw evaluation_exception(_positions[2], (boost::format("expected 1 argument for %1% but %2% were given.") % types::structure::name() % _arguments.size()).str());
            }

            // First argument should be a hash
            auto hash = move_parameter<values::hash>(0);
            if (!hash) {
                throw evaluation_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::hash::name() % get_type_name(_arguments[0])).str());
            }

            // Build a map of string -> Type
            unordered_map<string, values::type> types;
            for (auto& kvp : *hash) {
                // Ensure the key is a string
                auto str = boost::get<string>(&kvp.first);
                if (!str) {
                    throw evaluation_exception(_positions[0], (boost::format("expected hash keys to be %1% but found %2%.") % types::string::name() % get_type_name(kvp.first)).str());
                }
                // Ensure the value is a type
                auto type = boost::get<values::type>(&kvp.second);
                if (!type) {
                    throw evaluation_exception(_positions[0], (boost::format("expected hash values to be %1% but found %2%.") % types::type::name() % get_type_name(kvp.second)).str());
                }
                types.insert(make_pair(std::move(*str), std::move(*type)));
            }
            return types::structure(std::move(types));
        }

        template <typename T>
        result_type operator()(T const& target)
        {
            throw evaluation_exception(_position, (boost::format("access expression is not supported for %1% (%2%).") % get_type_name(target) % target).str());
        }

     private:
        template <typename Value>
        boost::optional<Value> move_parameter(size_t index)
        {
            // Check for a variable first
            auto var = dereference<Value>(_arguments[index]);
            if (var) {
                return var;
            }

            // Otherwise, check for the argument type
            auto result = boost::get<Value>(&_arguments[index]);
            if (!result) {
                return boost::none;
            }
            return std::move(*result);
        }

        template <typename Value, typename Type>
        tuple<Value, Value> get_range(bool accept_range = false, size_t start_index = 0) const
        {
            // Check for Integer type first
            if (accept_range && _arguments.size() > start_index) {
                auto type_ptr = boost::get<values::type>(&dereference(_arguments[start_index]));
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
                    auto ptr = boost::get<int64_t>(&dereference(argument));
                    if (ptr) {
                        from = static_cast<Value>(*ptr);
                    } else {
                        auto value_ptr = boost::get<Value>(&dereference(argument));
                        if (!value_ptr) {
                            throw evaluation_exception(_positions[start_index], (boost::format("expected parameter to be %1% but found %2%.") % Type::name() % get_type_name(argument)).str());
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
                    auto ptr = boost::get<int64_t>(&dereference(argument));
                    if (ptr) {
                        to = static_cast<Value>(*ptr);
                    } else {
                        auto value_ptr = boost::get<Value>(&dereference(argument));
                        if (!value_ptr) {
                            throw evaluation_exception(_positions[start_index], (boost::format("expected parameter to be %1% but found %2%.") % Type::name() % get_type_name(argument)).str());
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

    struct expression_visitor : boost::static_visitor<value>
    {
        explicit expression_visitor(expression_evaluator& evaluator) :
            _evaluator(evaluator)
        {
        }

        result_type operator()(boost::blank const&) const
        {
            return value();
        }

        result_type operator()(ast::basic_expression const& expr) const
        {
            return boost::apply_visitor(basic_expression_visitor(_evaluator), expr);
        }

        result_type operator()(ast::control_flow_expression const& expr) const
        {
            return boost::apply_visitor(control_flow_expression_visitor(_evaluator), expr);
        }

        result_type operator()(ast::catalog_expression const& expr) const
        {
            // TODO: implement
            throw evaluation_exception(get_position(expr), "catalog expression not yet implemented.");
        }

        result_type operator()(ast::unary_expression const& expr) const
        {
            switch (expr.op()) {
                case ast::unary_operator::negate:
                    return negate(boost::apply_visitor(*this, expr.operand()), expr.position());

                case ast::unary_operator::logical_not:
                    return logical_not(boost::apply_visitor(*this, expr.operand()));

                case ast::unary_operator::splat:
                    return splat(boost::apply_visitor(*this, expr.operand()));

                default:
                    throw evaluation_exception(expr.position(), "unexpected unary expression.");
            }
        }

        result_type operator()(ast::access_expression const& expr) const
        {
            value target = boost::apply_visitor(*this, expr.target());
            for (auto& access : expr.accesses()) {
                access_expression_evaluator visitor(_evaluator, access.arguments(), ast::get_position(expr.target()));
                target = boost::apply_visitor(visitor, dereference(target));
            }
            return target;
        }

        result_type operator()(ast::expression const& expr) const
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

        // Evaluate the first sub-expression
        auto result = evaluate(expr.first());
        auto position = expr.position();

        // Climb the remainder of the expression
        auto& remainder = expr.remainder();
        auto begin = remainder.begin();
        climb_expression(result, position, 0, begin, remainder.end());

        return result;
    }

    value expression_evaluator::evaluate(ast::primary_expression const& expr)
    {
        return boost::apply_visitor(expression_visitor(*this), expr);
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
        // An unfold expression is always unary
        if (!expr.remainder().empty()) {
            return boost::none;
        }
        // Unfold the first expression
        return unfold(expr.first(), result);
    }

    boost::optional<values::array> expression_evaluator::unfold(ast::primary_expression const& expression, value& evaluated)
    {
        // Determine if the given expression is a unary splat
        auto unary = boost::get<ast::unary_expression>(&expression);
        if (unary && unary->op() == ast::unary_operator::splat) {
            // If an array, return the array itself
            auto array_ptr = boost::get<values::array>(&evaluated);
            if (array_ptr) {
                return std::move(*array_ptr);
            }
            // Otherwise, it should be a variable
            if (!boost::get<variable>(&evaluated)) {
                return boost::none;
            }
            auto const_array_ptr = boost::get<values::array>(&dereference(evaluated));
            if (!const_array_ptr) {
                return boost::none;
            }
            // Return a copy of the array
            return *const_array_ptr;
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
        // Catalog and control flow expressions are productive
        if (boost::get<ast::catalog_expression>(&expr.first()) ||
            boost::get<ast::control_flow_expression>(&expr.first())) {
            return true;
        }

        // Expressions followed by an assignment or relationship operator are productive
        for (auto const& binary_expr : expr.remainder()) {
            if (binary_expr.op() == ast::binary_operator::assignment ||
                binary_expr.op() == ast::binary_operator::in_edge ||
                binary_expr.op() == ast::binary_operator::in_edge_subscribe ||
                binary_expr.op() == ast::binary_operator::out_edge ||
                binary_expr.op() == ast::binary_operator::out_edge_subscribe) {
                return true;
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
        switch (op) {
            case ast::binary_operator::in:
                left = in(left, right, _context);
                return;

            case ast::binary_operator::assignment:
                assign(left, right, _context, left_position);
                return;

            case ast::binary_operator::plus:
                left = plus(left, right, left_position, right_position);
                return;

            case ast::binary_operator::minus:
                left = minus(left, right, left_position, right_position);
                return;

            case ast::binary_operator::multiply:
                left = multiply(left, right, left_position, right_position);
                return;

            case ast::binary_operator::divide:
                left = divide(left, right, left_position, right_position);
                return;

            case ast::binary_operator::modulo:
                left = modulo(left, right, left_position, right_position);
                return;

            case ast::binary_operator::left_shift:
                left = left_shift(left, right, left_position, right_position);
                return;

            case ast::binary_operator::right_shift:
                left = right_shift(left, right, left_position, right_position);
                return;

            case ast::binary_operator::logical_and:
                left = logical_and(left, right);
                return;

            case ast::binary_operator::logical_or:
                left = logical_or(left, right);
                return;

            case ast::binary_operator::equals:
                left = equals(left, right);
                return;

            case ast::binary_operator::not_equals:
                left = !equals(left, right);
                return;

            case ast::binary_operator::less_than:
                left = less(left, right, left_position, right_position);
                return;

            case ast::binary_operator::less_equals:
                left = less_equal(left, right, left_position, right_position);
                return;

            case ast::binary_operator::greater_than:
                left = greater(left, right, left_position, right_position);
                return;

            case ast::binary_operator::greater_equals:
                left = greater_equal(left, right, left_position, right_position);
                return;

            case ast::binary_operator::match:
                left = match(left, right, left_position, right_position, _context);
                return;

            case ast::binary_operator::not_match:
                left = !is_truthy(match(left, right, left_position, right_position, _context));
                return;

            default:
                throw evaluation_exception(left_position, "unexpected binary expression.");
        }
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
