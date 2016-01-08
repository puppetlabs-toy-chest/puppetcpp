#include <puppet/compiler/evaluation/access_evaluator.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <utf8.h>

using namespace std;
using namespace puppet::compiler::ast;
using namespace puppet::runtime;
using namespace puppet::runtime::values;
using namespace puppet::runtime::types;

namespace puppet { namespace compiler { namespace evaluation {

    struct access_visitor : boost::static_visitor<value>
    {
        access_visitor(evaluation::context& context, ast::access_expression const& expression) :
            _context(context),
            _expression(expression)
        {
            evaluation::evaluator evaluator { _context };

            _arguments.reserve(expression.arguments.size());
            _contexts.reserve(expression.arguments.size());

            // Evaluate the arguments
            for (auto& argument : expression.arguments) {
                auto value = evaluator.evaluate(argument);

                // Check for argument splat
                if (argument.is_splat() && value.as<values::array>()) {
                    auto unfolded = value.move_as<values::array>();
                    _contexts.insert(_contexts.end(), unfolded.size(), argument.context());
                    _arguments.reserve(_arguments.size() + unfolded.size());
                    _arguments.insert(_arguments.end(), std::make_move_iterator(unfolded.begin()), std::make_move_iterator(unfolded.end()));
                    continue;
                }
                _contexts.push_back(argument.context());
                _arguments.emplace_back(rvalue_cast(value));
            }
        }

        value operator()(std::string const& target)
        {
            if (_arguments.size() > 2) {
                throw evaluation_exception((boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::string::name() % _arguments.size()).str(), _contexts[2]);
            }

            // Get the index
            auto ptr = _arguments[0]->as<int64_t>();
            if (!ptr) {
                throw evaluation_exception((boost::format("expected %1% for start index but found %2%.") % integer::name() % _arguments[0]->get_type()).str(), _contexts[0]);
            }

            // If the index is negative, it's from the end of the string
            int64_t index = *ptr;
            if (index < 0) {
                index += static_cast<int64_t>(target.size());
            }

            // Get the count
            int64_t count = 1;
            if (_arguments.size() == 2) {
                ptr = _arguments[1]->as<int64_t>();
                if (!ptr) {
                    throw evaluation_exception((boost::format("expected %1% for count but found %2%.") % integer::name() % _arguments[1]->get_type()).str(), _contexts[1]);
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

            if (target.empty() || static_cast<size_t>(index) >= target.size() || count <= 0) {
                return std::string();
            }

            // Iterate as Unicode characters
            auto begin = target.begin();
            for (int64_t i = 0; i < index && begin != target.end(); ++i) {
                utf8::next(begin, target.end());
            }
            auto end = begin;
            for (int64_t i = 0; i < count && end != target.end(); ++i) {
                utf8::next(end, target.end());
            }
            return std::string(begin, end);
        }

        value operator()(values::array const& target)
        {
            if (_arguments.size() > 2) {
                throw evaluation_exception((boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::array::name() % _arguments.size()).str(), _contexts[2]);
            }

            // Get the index
            auto ptr = _arguments[0]->as<int64_t>();
            if (!ptr) {
                throw evaluation_exception((boost::format("expected %1% for start index but found %2%.") % integer::name() % _arguments[0]->get_type()).str(), _contexts[0]);
            }

            // If the index is negative, it's from the end of the array
            int64_t index = *ptr;
            if (index < 0) {
                index += static_cast<int64_t>(target.size());
            }

            // Get the count
            int64_t count = 1;
            if (_arguments.size() == 2) {
                ptr = _arguments[1]->as<int64_t>();
                if (!ptr) {
                    throw evaluation_exception((boost::format("expected %1% for count but found %2%.") % integer::name() % _arguments[1]->get_type()).str(), _contexts[1]);
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

        value operator()(values::hash const& target)
        {
            if (_arguments.size() == 1) {
                // Lookup by key
                auto value = target.get(_arguments[0]);
                return value ? *value : values::undef();
            }

            // Otherwise, build an array of values
            values::array result;
            for (auto const& argument : _arguments) {
                // Lookup by key
                auto value = target.get(argument);
                if (!value) {
                    continue;
                }
                result.emplace_back(*value);
            }
            return result;
        }

        value operator()(values::type const& target)
        {
            return boost::apply_visitor(*this, target);
        }

        value operator()(integer const& target)
        {
            // At most 2 arguments to Integer
            if (_arguments.size() > 2) {
                throw evaluation_exception((boost::format("expected at most 2 arguments for %1% but %2% were given.") % integer::name() % _arguments.size()).str(), _contexts[2]);
            }

            int64_t from, to;
            tie(from, to) = get_range<int64_t, integer>();
            return integer(from, to);
        }

        value operator()(floating const& target)
        {
            // At most 2 arguments to Float
            if (_arguments.size() > 2) {
                throw evaluation_exception((boost::format("expected at most 2 arguments for %1% but %2% were given.") % floating::name() % _arguments.size()).str(), _contexts[2]);
            }

            double from, to;
            tie(from, to) = get_range<double, floating>();
            return floating(from, to);
        }

        value operator()(types::string const& target)
        {
            // At most 2 arguments to String
            if (_arguments.size() > 2) {
                throw evaluation_exception((boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::string::name() % _arguments.size()).str(), _contexts[2]);
            }

            int64_t from, to;
            tie(from, to) = get_range<int64_t, integer>(true);
            return types::string(from, to);
        }

        value operator()(regexp const& target)
        {
            // At most 1 arguments to Regexp
            if (_arguments.size() > 1) {
                throw evaluation_exception((boost::format("expected at most 1 arguments for %1% but %2% were given.") % regexp::name() % _arguments.size()).str(), _contexts[1]);
            }

            // Get the pattern argument; check for regex argument first
            std::string pattern;
            auto regex = _arguments[0]->as<values::regex>();
            if (regex) {
                pattern = regex->pattern();
            } else {
                if (!_arguments[0]->as<std::string>()) {
                    throw evaluation_exception((boost::format("expected parameter to be %1% or %2% but found %3%.") % types::string::name() % regexp::name() % _arguments[0]->get_type()).str(), _contexts[0]);
                }
                pattern = _arguments[0]->move_as<std::string>();
            }
            return regexp(rvalue_cast(pattern));
        }

        value operator()(enumeration const& target)
        {
            // Ensure each argument is a string
            vector<std::string> strings;
            strings.reserve(_arguments.size());

            for (size_t i = 0; i < _arguments.size(); ++i) {
                if (!_arguments[i]->as<std::string>()) {
                    throw evaluation_exception((boost::format("expected %1% but found %2%.") % types::string::name() % _arguments[i]->get_type()).str(), _contexts[i]);
                }
                strings.emplace_back(_arguments[i]->move_as<std::string>());
            }
            return enumeration(rvalue_cast(strings));
        }

        value operator()(pattern const& target)
        {
            vector<values::regex> patterns;
            patterns.reserve(_arguments.size());

            // Each argument can be a string, regex value, Regexp type or another Pattern type
            for (size_t i = 0; i < _arguments.size(); ++i) {
                // Check for string
                if (_arguments[i]->as<std::string>()) {
                    patterns.emplace_back(_arguments[i]->move_as<std::string>());
                    continue;
                }
                // Check for regex
                if (_arguments[i]->as<values::regex>()) {
                    patterns.emplace_back(_arguments[i]->move_as<values::regex>());
                    continue;
                }
                // Check for Type
                auto type = _arguments[i]->as<values::type>();
                if (type) {
                    auto regexp = boost::get<types::regexp>(type);
                    if (regexp) {
                        patterns.emplace_back(regexp->pattern());
                        continue;
                    }
                    // Check for Pattern type
                    auto pattern = boost::get<types::pattern>(type);
                    if (pattern) {
                        patterns.reserve(patterns.size() + pattern->patterns().size());
                        patterns.insert(patterns.end(), pattern->patterns().begin(), pattern->patterns().end());
                        continue;
                    }
                }
                throw evaluation_exception(
                    (boost::format("expected %1%, %2%, or %3% but found %4%.") %
                         types::string::name() %
                         regexp::name() %
                         pattern::name() %
                         _arguments[i]->get_type()
                    ).str(),
                    _contexts[i]);
            }
            return pattern(rvalue_cast(patterns));
        }

        value operator()(types::array const& target)
        {
            // At most 3 arguments to Array
            if (_arguments.size() > 3) {
                throw evaluation_exception((boost::format("expected at most 3 arguments for %1% but %2% were given.") % types::array::name() % _arguments.size()).str(), _contexts[3]);
            }

            // First argument should be a type
            if (!_arguments[0]->as<values::type>()) {
                throw evaluation_exception((boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % _arguments[0]->get_type()).str(), _contexts[0]);
            }

            // Get the optional range
            size_t from, to;
            tie(from, to) = get_range<int64_t, integer>(true, 1);
            return types::array(make_unique<values::type>(_arguments[0]->move_as<values::type>()), from, to);
        }

        value operator()(types::hash const& target)
        {
            // At least 2 and at most 4 arguments to Hash
            if (_arguments.size() < 2) {
                throw evaluation_exception((boost::format("expected at least 2 arguments for %1% but %2% were given.") % types::hash::name() % _arguments.size()).str(), _expression);
            }
            if (_arguments.size() > 4) {
                throw evaluation_exception((boost::format("expected at most 3 arguments for %1% but %2% were given.") % types::hash::name() % _arguments.size()).str(), _contexts[4]);
            }

            // First argument should be a type
            if (!_arguments[0]->as<values::type>()) {
                throw evaluation_exception((boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % _arguments[0]->get_type()).str(), _contexts[0]);
            }

            // Second argument should be a type
            if (!_arguments[1]->as<values::type>()) {
                throw evaluation_exception((boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % _arguments[1]->get_type()).str(), _contexts[1]);
            }

            // Get the optional range
            size_t from, to;
            tie(from, to) = get_range<int64_t, integer>(true, 2);
            return types::hash(
                make_unique<values::type>(_arguments[0]->move_as<values::type>()),
                make_unique<values::type>(_arguments[1]->move_as<values::type>()),
                from,
                to);
        }

        value operator()(types::tuple const& target)
        {
            vector<unique_ptr<values::type>> types;
            types.reserve(_arguments.size());

            int64_t from = _arguments.size();
            int64_t to = _arguments.size();
            for (size_t i = 0; i < _arguments.size(); ++i) {
                // Stop at first parameter that isn't a type
                if (!_arguments[i]->as<values::type>()) {
                    // There must be at most 2 more parameters (the range)
                    if ((i + 2) < _arguments.size()) {
                        throw evaluation_exception((boost::format("expected at most %1% arguments for %2% but %3% were given.") % (i + 2) % types::tuple::name() % _arguments.size()).str(), _contexts[i + 2]);
                    }
                    // Get the optional range
                    tie(from, to) = get_range<int64_t, integer>(false, i, 0, numeric_limits<int64_t>::max());
                    break;
                }
                types.emplace_back(new values::type(_arguments[i]->move_as<values::type>()));
            }
            if (types.empty()) {
                throw evaluation_exception((boost::format("expected %1% for first argument but found %2%.") % types::type::name() % _arguments[0]->get_type()).str(), _contexts[0]);
            }
            return types::tuple(rvalue_cast(types), from, to);
        }

        value operator()(optional const& target)
        {
            // Only 1 argument to Optional
            if (_arguments.size() > 1) {
                throw evaluation_exception((boost::format("expected 1 argument for %1% but %2% were given.") % optional::name() % _arguments.size()).str(), _contexts[2]);
            }

            // Check for type argument
            if (_arguments[0]->as<values::type>()) {
                return types::optional(make_unique<values::type>(_arguments[0]->move_as<values::type>()));
            }
            // Check for string argument (treat as Optional[Enum[<string>]])
            if (_arguments[0]->as<std::string>()) {
                vector<std::string> values;
                values.emplace_back(_arguments[0]->move_as<std::string>());
                return types::optional(make_unique<values::type>(types::enumeration(rvalue_cast(values))));
            }
            throw evaluation_exception(
                (boost::format("expected parameter to be %1% or %2% but found %3%.") %
                 types::type::name() %
                 types::string::name() %
                 _arguments[0]->get_type()
                ).str(),
                _contexts[0]);
        }

        value operator()(not_undef const& target)
        {
            // Only 1 argument to NotUndef
            if (_arguments.size() > 1) {
                throw evaluation_exception((boost::format("expected 1 argument for %1% but %2% were given.") % optional::name() % _arguments.size()).str(), _contexts[2]);
            }

            // Check for type argument
            if (_arguments[0]->as<values::type>()) {
                return types::not_undef(make_unique<values::type>(_arguments[0]->move_as<values::type>()));
            }
            // Check for string argument (treat as Optional[Enum[<string>]])
            if (_arguments[0]->as<std::string>()) {
                vector<std::string> values;
                values.emplace_back(_arguments[0]->move_as<std::string>());
                return types::not_undef(make_unique<values::type>(types::enumeration(rvalue_cast(values))));
            }
            throw evaluation_exception(
                (boost::format("expected parameter to be %1% or %2% but found %3%.") %
                 types::type::name() %
                 types::string::name() %
                 _arguments[0]->get_type()
                ).str(),
                _contexts[0]);
        }

        value operator()(types::type const& target)
        {
            // Only 1 argument to Type
            if (_arguments.size() > 1) {
                throw evaluation_exception((boost::format("expected 1 argument for %1% but %2% were given.") % types::type::name() % _arguments.size()).str(), _contexts[2]);
            }

            // First argument should be a type
            if (!_arguments[0]->as<values::type>()) {
                throw evaluation_exception((boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % _arguments[0]->get_type()).str(), _contexts[0]);
            }

            return types::type(make_unique<values::type>(_arguments[0]->move_as<values::type>()));
        }

        value operator()(structure const& target)
        {
            // Only 1 argument to Struct
            if (_arguments.size() > 1) {
                throw evaluation_exception((boost::format("expected 1 argument for %1% but %2% were given.") % structure::name() % _arguments.size()).str(), _contexts[2]);
            }

            // First argument should be a hash
            if (!_arguments[0]->as<values::hash>()) {
                throw evaluation_exception((boost::format("expected parameter to be %1% but found %2%.") % types::hash::name() % _arguments[0]->get_type()).str(), _contexts[0]);
            }

            auto hash = _arguments[0]->move_as<values::hash>();

            // Build a vector of key value pairs for the structure's schema
            structure::schema_type schema;
            for (auto& kvp : hash) {
                unique_ptr<values::type> key;
                if (auto ptr = kvp.key().as<std::string>()) {
                    // Optional key that is a string
                    key = make_unique<values::type>(enumeration({ *ptr }));
                } else if (auto type = kvp.key().as<values::type>()) {
                    // Otherwise, check for Optional[Enum[string]] and NotUndef[Enum[string]]
                    types::enumeration const* enumeration = nullptr;
                    if (auto optional = boost::get<types::optional>(type)) {
                        if (optional->type()) {
                            enumeration = boost::get<types::enumeration>(optional->type().get());
                        }
                    } else if (auto not_undef = boost::get<types::not_undef>(type)) {
                        if (not_undef->type()) {
                            enumeration = boost::get<types::enumeration>(not_undef->type().get());
                        }
                    }
                    if (enumeration && enumeration->strings().size() == 1) {
                        key = make_unique<values::type>(*type);
                    }
                }
                if (!key) {
                    throw evaluation_exception(
                        (boost::format("expected hash keys to be a non-empty %1%, %2%, or %3% but found %4%.") %
                         types::string::name() %
                         types::optional::name() %
                         types::not_undef::name() %
                         kvp.key().get_type()
                        ).str(),
                        _contexts[0]);
                }

                // Ensure the value is a type
                if (!kvp.value().as<values::type>()) {
                    throw evaluation_exception((boost::format("expected hash values to be %1% but found %2%.") % types::type::name() % kvp.value().get_type()).str(), _contexts[0]);
                }
                schema.emplace_back(make_pair(rvalue_cast(key), make_unique<values::type>(kvp.value().move_as<values::type>())));
            }
            return structure(rvalue_cast(schema));
        }

        value operator()(variant const& target)
        {
            vector<unique_ptr<values::type>> types;
            types.reserve(_arguments.size());

            for (size_t i = 0; i < _arguments.size(); ++i) {
                if (!_arguments[i]->as<values::type>()) {
                    throw evaluation_exception((boost::format("expected parameter to be %1% but found %2%.") % types::type::name() % _arguments[i]->get_type()).str(), _contexts[i]);
                }
                types.emplace_back(make_unique<values::type>(_arguments[i]->move_as<values::type>()));
            }
            return variant(rvalue_cast(types));
        }

        value operator()(types::resource const& target)
        {
            // If the type is fully qualified, then this is an attribute access
            if (target.fully_qualified()) {
                return access_resource(target);
            }

            // If the resource doesn't have a type, the first argument should be the type
            size_t offset = 0;
            auto type_name = target.type_name();
            if (type_name.empty()) {
                if (_arguments[0]->as<std::string>()) {
                    type_name = _arguments[0]->move_as<std::string>();
                } else if (auto type = _arguments[0]->as<values::type>()) {
                    if (auto resource = boost::get<types::resource>(type)) {
                        type_name = resource->type_name();
                    }
                }
                offset = 1;
            }
            if (type_name.empty()) {
                throw evaluation_exception((boost::format("expected parameter to be %1% or typed %2% but found %3%.") % types::string::name() % types::resource::name() % _arguments[0]->get_type()).str(), _contexts[0]);
            }

            // Check for Resource['typename']
            if (_arguments.size() == offset) {
                return types::resource(type_name);
            }

            // If there is only one additional string parameter, return a single resource
            if (_arguments.size() == (offset + 1) && _arguments[offset]->as<std::string>()) {
                return types::resource(type_name, _arguments[offset]->move_as<std::string>());
            }

            // Otherwise, return an array of resources with titles
            values::array result;
            for (size_t i = offset; i < _arguments.size(); ++i) {
                add_resource_reference(result, type_name, _arguments[i], _contexts[i]);
            }
            return result;
        }

        value operator()(types::klass const& target)
        {
            // If the type is fully qualified, then this is an attribute access
            if (target.fully_qualified()) {
                return access_resource(types::resource("class", target.title()));
            }

            // If there is only one string parameter, return a single class
            if (_arguments.size() == 1 && _arguments[0]->as<std::string>()) {
                return types::klass(_arguments[0]->move_as<std::string>());
            }

            // Otherwise, return an array of classes with titles
            values::array result;
            for (size_t i = 0; i < _arguments.size(); ++i) {
                add_class_reference(result, _arguments[i], _contexts[i]);
            }
            return result;
        }

        value operator()(callable const& target)
        {
            vector<unique_ptr<values::type>> types;
            types.reserve(_arguments.size());

            int64_t min = _arguments.size();
            int64_t max = _arguments.size();
            unique_ptr<values::type> block_type;
            for (size_t i = 0; i < _arguments.size(); ++i) {
                // Stop at first parameter that isn't a type
                if (!_arguments[i]->as<values::type>()) {
                    // There must be at most 3 more parameters (min, max, and block type)
                    if ((i + 3) < _arguments.size()) {
                        throw evaluation_exception((boost::format("expected at most %1% arguments for %2% but %3% were given.") % (i + 3) % types::callable::name() % _arguments.size()).str(), _contexts[i + 3]);
                    }
                    // Get the optional min/max range
                    tie(min, max) = get_range<int64_t, integer>(false, i, 0, numeric_limits<int64_t>::max());

                    // Get the optional block type
                    if (i + 2 < _arguments.size()) {
                        // Ensure the last argument is a type representing the block's signature
                        if (_arguments[i + 2]->as<values::type>()) {
                            auto type = _arguments[i + 2]->move_as<values::type>();

                            bool acceptable = false;

                            // Check for just Callable
                            if (boost::get<callable>(&type)) {
                                acceptable = true;
                            } else if (auto optional = boost::get<types::optional>(&type)) {
                                // Check the Optional[Callable]
                                if (optional->type() && boost::get<callable>(optional->type().get())) {
                                    acceptable = true;
                                }
                            }

                            // Move the type into a new type if it is acceptable
                            if (acceptable) {
                                block_type.reset(new values::type{ rvalue_cast(type) });
                            }
                        }
                        if (!block_type) {
                            throw evaluation_exception((boost::format("expected %1% or %2%[%1%] for last argument but found %3%.") % types::callable::name() % types::optional::name() % _arguments[i + 2]->get_type()).str(), _contexts[i + 2]);
                        }
                    }
                    break;
                }
                types.emplace_back(new values::type(_arguments[i]->move_as<values::type>()));
            }
            return callable(rvalue_cast(types), min, max, rvalue_cast(block_type));
        }

        value operator()(types::runtime const& target)
        {
            // Ensure there are at most 2 arguments to runtime
            if (_arguments.size() > 2) {
                throw evaluation_exception((boost::format("expected at most 2 arguments for %1% but %2% were given.") % types::runtime::name() % _arguments.size()).str(), _contexts[2]);
            }

            // First argument should be a string
            if (!_arguments[0]->as<std::string>()) {
                throw evaluation_exception((boost::format("expected parameter to be %1% but found %2%.") % types::string::name() % _arguments[0]->get_type()).str(), _contexts[0]);
            }
            auto runtime_name = _arguments[0]->move_as<std::string>();

            // Check for the optional type name
            std::string type_name;
            if (_arguments.size() > 1) {
                if (!_arguments[1]->as<std::string>()) {
                    throw evaluation_exception((boost::format("expected parameter to be %1% but found %2%.") % types::string::name() % _arguments[1]->get_type()).str(), _contexts[1]);
                }
                type_name = _arguments[1]->move_as<std::string>();
            }
            return types::runtime(rvalue_cast(runtime_name), rvalue_cast(type_name));
        }

        template <typename T>
        value operator()(T const& target)
        {
            throw evaluation_exception((boost::format("access expression is not supported for %1%.") % value(target).get_type()).str(), _expression);
        }

        template <typename Value, typename Type>
        std::tuple<Value, Value> get_range(
            bool accept_range = false,
            size_t start_index = 0,
            Value from_default = numeric_limits<Value>::min(),
            Value to_default = numeric_limits<Value>::max()) const
        {
            // Check for Integer range first
            if (accept_range && _arguments.size() > start_index) {
                if (auto type = _arguments[start_index]->as<values::type>()) {
                    if (auto integer = boost::get<types::integer>(type)) {
                        if ((start_index + 1) < _arguments.size()) {
                            // Ranges must be the last argument
                            throw evaluation_exception((boost::format("an %1% range must be the last argument.") % types::integer::name()).str(), _contexts[start_index]);
                        }
                        return make_tuple(integer->from(), integer->to());
                    }
                }
            }

            // Get the from argument
            auto from = from_default;
            if (_arguments.size() > start_index) {
                auto& argument = _arguments[start_index];

                if (!argument->is_default()) {
                    // Try int64_t first; this allows either Integer or Floats when Value is floating point
                    if (auto ptr = argument->as<int64_t>()) {
                        from = static_cast<Value>(*ptr);
                    } else if (auto ptr = argument->as<Value>()) {
                        from = *ptr;
                    } else {
                        throw evaluation_exception((boost::format("expected parameter to be %1% but found %2%.") % Type::name() % argument->get_type()).str(), _contexts[start_index]);
                    }
                }
            }
            // Get the to argument
            auto to = to_default;
            if (_arguments.size() > (start_index + 1)) {
                auto& argument = _arguments[start_index + 1];

                if (!argument->is_default()) {
                    // Try int64_t first; this allows either Integer or Floats when Value is floating point
                    if (auto ptr = argument->as<int64_t>()) {
                        to = static_cast<Value>(*ptr);
                    } else if (auto ptr = argument->as<Value>()) {
                        to = *ptr;
                    } else {
                        throw evaluation_exception((boost::format("expected parameter to be %1% but found %2%.") % Type::name() % argument->get_type()).str(), _contexts[start_index + 1]);
                    }
                }
            }
            return make_tuple(from, to);
        }

        void add_resource_reference(values::array& result, std::string const& type_name, value& argument, ast::context const& context)
        {
            if (argument.as<std::string>()) {
                result.emplace_back(types::resource(type_name, argument.move_as<std::string>()));
            } else if (argument.as<values::array>()) {
                auto titles = argument.move_as<values::array>();
                for (auto& element : titles) {
                    add_resource_reference(result, type_name, element, context);
                }
            } else {
                throw evaluation_exception((boost::format("expected %1% for resource title but found %2%.") % types::string::name() % argument.get_type()).str(), context);
            }
        }

        void add_class_reference(values::array& result, value& argument, ast::context const& context)
        {
            if (argument.as<std::string>()) {
                result.emplace_back(types::klass(argument.move_as<std::string>()));
            } else if (argument.as<values::array>()) {
                auto titles = argument.move_as<values::array>();
                for (auto& element : titles) {
                    add_class_reference(result, element, context);
                }
            } else {
                throw evaluation_exception((boost::format("expected %1% for class title but found %2%.") % types::string::name() % argument.get_type()).str(), context);
            }
        }

        value access_resource(types::resource const& target)
        {
            auto& catalog = _context.catalog();

            // Find the resource
            auto resource = catalog.find(target);
            if (!resource) {
                throw evaluation_exception((boost::format("resource %1% does not exist in the catalog.") % target).str(), _expression);
            }

            // Check for single access
            if (_arguments.size() == 1) {
                return access_attribute(*resource, 0);
            }

            // Lookup each argument as an attribute
            values::array attributes;
            for (size_t i = 0; i < _arguments.size(); ++i) {
                attributes.emplace_back(access_attribute(*resource, i));
            }
            return attributes;
        }

        value access_attribute(compiler::resource const& resource, size_t index)
        {
            if (!_arguments[index]->as<std::string>()) {
                throw evaluation_exception((boost::format("expected parameter to be %1% but found %2%.") % types::string::name() % _arguments[index]->get_type()).str(), _contexts[index]);
            }
            // Lookup the attribute
            auto name = _arguments[index]->move_as<std::string>();
            auto attribute = resource.get(name);
            if (!attribute) {
                throw evaluation_exception((boost::format("resource %1% does not have an attribute named '%2%'.") % resource.type() % name).str(), _contexts[index]);
            }
            // Treat the value as a variable so we don't needlessly copy the value
            return values::variable(rvalue_cast(name), attribute->shared_value());
        }

     private:
        evaluation::context& _context;
        access_expression const& _expression;
        values::array _arguments;
        vector<ast::context> _contexts;
    };

    access_evaluator::access_evaluator(evaluation::context& context) :
        _context(context)
    {
    }

    value access_evaluator::evaluate(value const& target, access_expression const& expression)
    {
        access_visitor visitor{ _context, expression };
        return boost::apply_visitor(visitor, target);
    }

}}}  // namespace puppet::compiler::evaluation
