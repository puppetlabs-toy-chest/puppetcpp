#include <puppet/runtime/evaluators/access.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace evaluators {

    access_expression_evaluator::access_expression_evaluator(expression_evaluator& evaluator, ast::access_expression const& expression) :
        _evaluator(evaluator),
        _expression(expression)
    {
        // Evaluate all the expressions for the access
        for (auto& argument : expression.arguments()) {
            auto value = _evaluator.evaluate(argument);

            // If unfolding, append the array's elements
            auto unfold_array = _evaluator.unfold(argument, value);
            if (unfold_array) {
                _positions.insert(_positions.end(), unfold_array->size(), argument.position());
                _arguments.reserve(_arguments.size() + unfold_array->size());
                _arguments.insert(_arguments.end(), std::make_move_iterator(unfold_array->begin()), std::make_move_iterator(unfold_array->end()));
                continue;
            }
            _positions.push_back(argument.position());
            _arguments.emplace_back(rvalue_cast(value));
        }
    }

    access_expression_evaluator::result_type access_expression_evaluator::evaluate(value const& target)
    {
        return boost::apply_visitor(*this, target);
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(string const& target)
    {
        if (_arguments.size() > 2) {
            throw _evaluator.create_exception(_positions[2], (boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::string::name() % _arguments.size()).str());
        }

        // Get the index
        auto ptr = as<int64_t>(_arguments[0]);
        if (!ptr) {
            throw _evaluator.create_exception(_positions[0], (boost::format("expected %1% for start index but found %2%.") % types::integer::name() % get_type(_arguments[0])).str());
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
                throw _evaluator.create_exception(_positions[1], (boost::format("expected %1% for count but found %2%.") % types::integer::name() % get_type(_arguments[1])).str());
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

    access_expression_evaluator::result_type access_expression_evaluator::operator()(values::array const& target)
    {
        if (_arguments.size() > 2) {
            throw _evaluator.create_exception(_positions[2], (boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::array::name() % _arguments.size()).str());
        }

        // Get the index
        auto ptr = as<int64_t>(_arguments[0]);
        if (!ptr) {
            throw _evaluator.create_exception(_positions[0], (boost::format("expected %1% for start index but found %2%.") % types::integer::name() % get_type(_arguments[0])).str());
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
                throw _evaluator.create_exception(_positions[1], (boost::format("expected %1% for count but found %2%.") % types::integer::name() % get_type(_arguments[1])).str());
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

    access_expression_evaluator::result_type access_expression_evaluator::operator()(values::hash const& target)
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

    access_expression_evaluator::result_type access_expression_evaluator::operator()(type const& target)
    {
        return boost::apply_visitor(*this, target);
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::integer const& type)
    {
        // At most 2 arguments to Integer
        if (_arguments.size() > 2) {
            throw _evaluator.create_exception(_positions[2], (boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::integer::name() % _arguments.size()).str());
        }

        int64_t from, to;
        tie(from, to) = get_range<int64_t, types::integer>();
        return types::integer(from, to);
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::floating const& type)
    {
        // At most 2 arguments to Float
        if (_arguments.size() > 2) {
            throw _evaluator.create_exception(_positions[2], (boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::floating::name() % _arguments.size()).str());
        }

        long double from, to;
        tie(from, to) = get_range<long double, types::floating>();
        return types::floating(from, to);
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::string const& type)
    {
        // At most 2 arguments to String
        if (_arguments.size() > 2) {
            throw _evaluator.create_exception(_positions[2], (boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::string::name() % _arguments.size()).str());
        }

        int64_t from, to;
        tie(from, to) = get_range<int64_t, types::integer>(true);
        return types::string(from, to);
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::regexp const& type)
    {
        // At most 1 arguments to Regexp
        if (_arguments.size() > 1) {
            throw _evaluator.create_exception(_positions[1], (boost::format("expected at most 1 arguments for %1% but %2% were given.") % types::regexp::name() % _arguments.size()).str());
        }

        // Get the pattern argument; check for regex argument first
        string pattern;
        auto regex = as<values::regex>(_arguments[0]);
        if (regex) {
            pattern = regex->pattern();
        } else {
            if (!as<string>(_arguments[0])) {
                throw _evaluator.create_exception(_positions[0], (boost::format("expected parameter to be %1% or %2% but found %3%.") % types::string::name() % types::regexp::name() % get_type(_arguments[0])).str());
            }
            pattern = mutate_as<string>(_arguments[0]);
        }
        return types::regexp(rvalue_cast(pattern));
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::enumeration const& type)
    {
        // Ensure each argument is a string
        vector<string> strings;
        strings.reserve(_arguments.size());

        for (size_t i = 0; i < _arguments.size(); ++i) {
            if (!as<string>(_arguments[i])) {
                throw _evaluator.create_exception(_positions[i], (boost::format("expected %1% but found %2%.") % types::string::name() % get_type(_arguments[i])).str());
            }
            strings.emplace_back(mutate_as<string>(_arguments[i]));
        }
        return types::enumeration(rvalue_cast(strings));
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::pattern const& type)
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
            throw _evaluator.create_exception(_positions[i], (boost::format("expected %1%, %2%, or %3% but found %4%.") %
                    types::string::name() %
                    types::regexp::name() %
                    types::pattern::name() %
                    get_type(_arguments[i])).str());
        }
        return types::pattern(rvalue_cast(patterns));
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::array const& type)
    {
        // At most 3 arguments to Array
        if (_arguments.size() > 3) {
            throw _evaluator.create_exception(_positions[3], (boost::format("expected at most 3 arguments for %1% but %2% were given.") % types::array::name() % _arguments.size()).str());
        }

        // First argument should be a type
        if (!as<values::type>(_arguments[0])) {
            throw _evaluator.create_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[0])).str());
        }

        // Get the optional range
        size_t from, to;
        tie(from, to) = get_range<int64_t, types::integer>(true, 1);
        return types::array(mutate_as<values::type>(_arguments[0]), from, to);
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::hash const& type)
    {
        // At least 2 and at most 4 arguments to Hash
        if (_arguments.size() < 2) {
            throw _evaluator.create_exception(_expression.position(), (boost::format("expected at least 2 arguments for %1% but %2% were given.") % types::hash::name() % _arguments.size()).str());
        }
        if (_arguments.size() > 4) {
            throw _evaluator.create_exception(_positions[4], (boost::format("expected at most 3 arguments for %1% but %2% were given.") % types::hash::name() % _arguments.size()).str());
        }

        // First argument should be a type
        if (!as<values::type>(_arguments[0])) {
            throw _evaluator.create_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[0])).str());
        }

        // Second argument should be a type
        if (!as<values::type>(_arguments[1])) {
            throw _evaluator.create_exception(_positions[1], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[1])).str());
        }

        // Get the optional range
        size_t from, to;
        tie(from, to) = get_range<int64_t, types::integer>(true, 2);
        return types::hash(mutate_as<values::type>(_arguments[0]), mutate_as<values::type>(_arguments[1]), from, to);
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::tuple const& type)
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
                    throw _evaluator.create_exception(_positions[i + 2], (boost::format("expected at most %1% arguments for %2% but %3% were given.") % (i + 2) % types::tuple::name() % _arguments.size()).str());
                }
                // Get the optional range
                tie(from, to) = get_range<int64_t, types::integer>(true, i);
                break;
            }
            types.emplace_back(mutate_as<values::type>(_arguments[i]));
        }
        return types::tuple(rvalue_cast(types), from, to);
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::optional const& type)
    {
        // Only 1 argument to Optional
        if (_arguments.size() > 1) {
            throw _evaluator.create_exception(_positions[2], (boost::format("expected 1 argument for %1% but %2% were given.") % types::optional::name() % _arguments.size()).str());
        }

        // First argument should be a type
        if (!as<values::type>(_arguments[0])) {
            throw _evaluator.create_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[0])).str());
        }

        return types::optional(mutate_as<values::type>(_arguments[0]));
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::type const& type)
    {
        // Only 1 argument to Type
        if (_arguments.size() > 1) {
            throw _evaluator.create_exception(_positions[2], (boost::format("expected 1 argument for %1% but %2% were given.") % types::type::name() % _arguments.size()).str());
        }

        // First argument should be a type
        if (!as<values::type>(_arguments[0])) {
            throw _evaluator.create_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[0])).str());
        }

        return types::type(mutate_as<values::type>(_arguments[0]));
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::structure const& type)
    {
        // Only 1 argument to Struct
        if (_arguments.size() > 1) {
            throw _evaluator.create_exception(_positions[2], (boost::format("expected 1 argument for %1% but %2% were given.") % types::structure::name() % _arguments.size()).str());
        }

        // First argument should be a hash
        if (!as<values::hash>(_arguments[0])) {
            throw _evaluator.create_exception(_positions[0], (boost::format("expected parameter to be %1% but found %2%.") % types::hash::name() % get_type(_arguments[0])).str());
        }

        auto hash = mutate_as<values::hash>(_arguments[0]);

        // Build a map of string -> Type
        unordered_map<string, values::type> types;
        for (auto& kvp : hash) {
            // Ensure the key is a string
            auto key = as<string>(kvp.first);
            if (!key) {
                throw _evaluator.create_exception(_positions[0], (boost::format("expected hash keys to be %1% but found %2%.") % types::string::name() % get_type(kvp.first)).str());
            }
            // Ensure the value is a type
            if (!as<values::type>(kvp.second)) {
                throw _evaluator.create_exception(_positions[0], (boost::format("expected hash values to be %1% but found %2%.") % types::type::name() % get_type(kvp.second)).str());
            }
            types.insert(make_pair(*key, mutate_as<values::type>(kvp.second)));
        }
        return types::structure(rvalue_cast(types));
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::variant const& type)
    {
        vector<values::type> types;
        types.reserve(_arguments.size());

        for (size_t i = 0; i < _arguments.size(); ++i) {
            if (!as<values::type>(_arguments[i])) {
                throw _evaluator.create_exception(_positions[i], (boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % get_type(_arguments[i])).str());
            }
            types.emplace_back(mutate_as<values::type>(_arguments[i]));
        }
        return types::variant(rvalue_cast(types));
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::resource const& type)
    {
        // If the resource doesn't have a type, the first argument should be the type
        size_t offset = 0;
        string type_name = type.type_name();
        if (type_name.empty()) {
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
            offset = 1;
        }
        if (type_name.empty()) {
            throw _evaluator.create_exception(_positions[0], (boost::format("expected parameter to be %1% or typed %2% but found %3%.") % types::string::name() % types::resource::name() % get_type(_arguments[0])).str());
        }

        // Check for Resource['typename']
        if (_arguments.size() == offset) {
            return types::resource(type_name);
        }

        // If there is only one additional string parameter, return a single resource
        if (_arguments.size() == (offset + 1) && as<string>(_arguments[offset])) {
            return types::resource(type_name, mutate_as<string>(_arguments[offset]));
        }

        // Otherwise, return an array of resources with titles
        values::array result;
        for (size_t i = offset; i < _arguments.size(); ++i) {
            add_resource_reference(result, type_name, _arguments[i], _positions[i]);
        }
        return result;
    }

    access_expression_evaluator::result_type access_expression_evaluator::operator()(types::klass const& type)
    {
        // If there is only one string parameter, return a single class
        if (_arguments.size() == 1 && as<string>(_arguments[0])) {
            return types::klass(mutate_as<string>(_arguments[0]));
        }

        // Otherwise, return an array of classes with titles
        values::array result;
        for (size_t i = 0; i < _arguments.size(); ++i) {
            add_class_reference(result, _arguments[i], _positions[i]);
        }
        return result;
    }

    void access_expression_evaluator::add_resource_reference(values::array& result, string const& type_name, values::value& argument, lexer::position const& position)
    {
        if (as<string>(argument)) {
            result.emplace_back(types::resource(type_name, mutate_as<string>(argument)));
        } else if (as<values::array>(argument)) {
            auto titles = mutate_as<values::array>(argument);
            for (auto& element : titles) {
                add_resource_reference(result, type_name, element, position);
            }
        } else {
            throw _evaluator.create_exception(position, (boost::format("expected %1% for resource title but found %2%.") % types::string::name() % get_type(argument)).str());
        }
    }

    void access_expression_evaluator::add_class_reference(values::array& result, values::value& argument, lexer::position const& position)
    {
        if (as<string>(argument)) {
            result.emplace_back(types::klass(mutate_as<string>(argument)));
        } else if (as<values::array>(argument)) {
            auto titles = mutate_as<values::array>(argument);
            for (auto& element : titles) {
                add_class_reference(result, element, position);
            }
        } else {
            throw _evaluator.create_exception(position, (boost::format("expected %1% for class title but found %2%.") % types::string::name() % get_type(argument)).str());
        }
    }

}}}  // namespace puppet::runtime::evaluators
