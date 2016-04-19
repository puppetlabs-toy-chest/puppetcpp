#include <puppet/runtime/values/value.hpp>
#include <puppet/compiler/evaluation/collectors/collector.hpp>
#include <puppet/utility/indirect_collection.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <rapidjson/document.h>
#include <utf8.h>

using namespace std;
using namespace rapidjson;
using namespace puppet::runtime;
using namespace puppet::compiler::evaluation;

namespace puppet { namespace runtime { namespace values {

    value::value(values::wrapper<value>&& wrapper) :
        value_base(rvalue_cast(static_cast<value_base&>(wrapper.get())))
    {
    }

    value::value(char const* string) :
        value_base(std::string(string))
    {
    }

    value& value::operator=(values::wrapper<value>&& wrapper)
    {
        value_base::operator=(rvalue_cast(static_cast<value_base&>(wrapper.get())));
        return *this;
    }

    value& value::operator=(char const* string)
    {
        value_base::operator=(std::string(string));
        return *this;
    }

    bool value::is_undef() const
    {
        return static_cast<bool>(as<undef>());
    }

    bool value::is_default() const
    {
        return static_cast<bool>(as<defaulted>());
    }

    bool value::is_true() const
    {
        auto ptr = as<bool>();
        return ptr && *ptr;
    }

    bool value::is_false() const
    {
        auto ptr = as<bool>();
        return ptr && !*ptr;
    }

    bool value::is_truthy() const
    {
        if (is_undef()) {
            return false;
        }
        if (auto ptr = as<bool>()) {
            return *ptr;
        }
        return true;
    }

    struct type_inference_visitor : boost::static_visitor<values::type>
    {
        result_type operator()(undef const&)
        {
            return types::undef{};
        }

        result_type operator()(defaulted const&)
        {
            return types::defaulted{};
        }

        result_type operator()(int64_t value)
        {
            return types::integer{ value, value };
        }

        result_type operator()(double value)
        {
            return types::floating{ value, value };
        }

        result_type operator()(bool)
        {
            return types::boolean{};
        }

        result_type operator()(string const& value)
        {
            return types::string{
                static_cast<int64_t>(value.size()),
                static_cast<int64_t>(value.size())
            };
        }

        result_type operator()(values::regex const& value)
        {
            return types::regexp{ value.pattern() };
        }

        result_type operator()(type const& t)
        {
            return types::type{ make_unique<type>(t) };
        }

        result_type operator()(variable const& var)
        {
            return boost::apply_visitor(*this, var.value());
        }

        result_type operator()(array const& value)
        {
            if (value.empty()) {
                return types::array{ nullptr, 0, 0 };
            }

            bool first = true;
            result_type element_type;
            for (auto& element : value) {
                if (first) {
                    element_type = boost::apply_visitor(*this, *element);
                    first = false;
                    continue;
                }

                element_type = infer_common_type(boost::apply_visitor(*this, *element), element_type);
            }
            return types::array{
                make_unique<type>(rvalue_cast(element_type)),
                static_cast<int64_t>(value.size()),
                static_cast<int64_t>(value.size())
            };
        }

        result_type operator()(hash const& value)
        {
            if (value.empty()) {
                return types::hash{ nullptr, nullptr, 0, 0 };
            }

            bool first = true;
            result_type key_type;
            result_type value_type;
            for (auto& kvp : value) {
                if (first) {
                    key_type = boost::apply_visitor(*this, kvp.key());
                    value_type = boost::apply_visitor(*this, kvp.value());
                    first = false;
                    continue;
                }
                key_type = infer_common_type(boost::apply_visitor(*this, kvp.key()), key_type);
                value_type = infer_common_type(boost::apply_visitor(*this, kvp.value()), value_type);
            }
            return types::hash{
                make_unique<type>(rvalue_cast(key_type)),
                make_unique<type>(rvalue_cast(value_type)),
                static_cast<int64_t>(value.size()),
                static_cast<int64_t>(value.size())
            };
        }

        result_type operator()(iterator const& value)
        {
            return types::iterator{ make_unique<type>(value.infer_produced_type()) };
        }

     private:
        result_type infer_common_type(result_type const& left, result_type const& right)
        {
            // Check for right is assignable to left and vice versa
            if (left.is_assignable(right, _guard)) {
                return left;
            }
            if (right.is_assignable(left, _guard)) {
                return right;
            }
            // Check for both Array
            if (auto left_array = boost::get<types::array>(&left)) {
                if (auto right_array = boost::get<types::array>(&right)) {
                    return types::array{
                        make_unique<values::type>(infer_common_type(left_array->element_type(), right_array->element_type()))
                    };
                }
            }
            // Check for both Hash
            if (auto left_hash = boost::get<types::hash>(&left)) {
                if (auto right_hash = boost::get<types::hash>(&right)) {
                    return types::hash{
                        make_unique<values::type>(infer_common_type(left_hash->key_type(), right_hash->key_type())),
                        make_unique<values::type>(infer_common_type(left_hash->value_type(), right_hash->value_type()))
                    };
                }
            }
            // Check for both Class
            if (boost::get<types::klass>(&left) && boost::get<types::klass>(&right)) {
                // Unparameterized Class is the common type because neither was assignable above
                return types::klass{};
            }
            // Check for both Resource
            if (auto left_resource = boost::get<types::resource>(&left)) {
                if (auto right_resource = boost::get<types::resource>(&right)) {
                    if (left_resource->type_name() == right_resource->type_name()) {
                        return types::resource{ left_resource->type_name() };
                    }
                    // Unparameterized Resource is the common type because neither was assignable above
                    return types::resource{};
                }
            }
            // Check for both Integer
            if (auto left_integer = boost::get<types::integer>(&left)) {
                if (auto right_integer = boost::get<types::integer>(&right)) {
                    return types::integer{
                        std::min(left_integer->from(), right_integer->from()),
                        std::max(left_integer->to(), right_integer->to())
                    };
                }
            }
            // Check for both Float
            if (auto left_float = boost::get<types::floating>(&left)) {
                if (auto right_float = boost::get<types::floating>(&right)) {
                    return types::floating{
                        std::min(left_float->from(), right_float->from()),
                        std::max(left_float->to(), right_float->to())
                    };
                }
            }
            // Check for both String
            if (auto left_string = boost::get<types::string>(&left)) {
                if (auto right_string = boost::get<types::string>(&right)) {
                    return types::string{
                        std::min(left_string->from(), right_string->from()),
                        std::max(left_string->to(), right_string->to())
                    };
                }
            }
            // Check for both Pattern
            if (auto left_pattern = boost::get<types::pattern>(&left)) {
                if (auto right_pattern = boost::get<types::pattern>(&right)) {
                    return types::pattern{ join_sets<values::regex>(left_pattern->patterns(), right_pattern->patterns()) };
                }
            }
            // Check for both Enum
            if (auto left_enum = boost::get<types::enumeration>(&left)) {
                if (auto right_enum = boost::get<types::enumeration>(&right)) {
                    return types::enumeration{ join_sets<std::string>(left_enum->strings(), right_enum->strings())};
                }
            }
            // Check for both Variant
            if (auto left_variant = boost::get<types::variant>(&left)) {
                if (auto right_variant = boost::get<types::variant>(&right)) {
                    return types::variant{ join_sets<values::type>(left_variant->types(), right_variant->types()) };
                }
            }
            // Check for both Type
            if (auto left_type = boost::get<types::type>(&left)) {
                if (auto right_type = boost::get<types::type>(&right)) {
                    if (!left_type->parameter() || !right_type->parameter()) {
                        return types::type{};
                    }
                    return types::type{
                        make_unique<values::type>(infer_common_type(*left_type->parameter(), *right_type->parameter()))
                    };
                }
            }
            // Check for both Regexp
            if (boost::get<types::regexp>(&left) && boost::get<types::regexp>(&right)) {
                // Unparameterized Regexp is the common type because neither was assignable above
                return types::regexp{};
            }
            // Check for both Callable
            if (boost::get<types::callable>(&left) && boost::get<types::callable>(&right)) {
                // Unparameterized Callable is the common type because neither was assignable above
                return types::callable{};
            }
            // Check for both Runtime
            if (boost::get<types::runtime>(&left) && boost::get<types::runtime>(&right)) {
                // Unparameterized Runtime is the common type because neither was assignable above
                return types::runtime{};
            }
            // Check fror both Numeric
            if (types::numeric::instance.is_assignable(left, _guard) && types::numeric::instance.is_assignable(right, _guard)) {
                return types::numeric{};
            }
            // Check fror both Scalar
            if (types::scalar::instance.is_assignable(left, _guard) && types::scalar::instance.is_assignable(right, _guard)) {
                return types::scalar{};
            }
            // Check fror both Data
            if (types::data::instance.is_assignable(left, _guard) && types::data::instance.is_assignable(right, _guard)) {
                return types::data{};
            }

            // None of the above, return Any
            return types::any{};
        }

        template <typename T>
        static vector<T> join_sets(vector<T> const& left, vector<T> const& right)
        {
            vector<T> result;
            utility::indirect_set<T> set;
            for (auto& element : left) {
                if (set.insert(&element).second) {
                    result.push_back(element);
                }
            }
            for (auto& element : right) {
                if (set.insert(&element).second) {
                    result.push_back(element);
                }
            }
            return result;
        }

        template <typename T>
        static vector<unique_ptr<T>> join_sets(vector<unique_ptr<T>> const& left, vector<unique_ptr<T>> const& right)
        {
            vector<unique_ptr<T>> result;
            utility::indirect_set<T> set;
            for (auto& element : left) {
                if (set.insert(element.get()).second) {
                    result.push_back(make_unique<T>(*element));
                }
            }
            for (auto& element : right) {
                if (set.insert(element.get()).second) {
                    result.push_back(make_unique<T>(*element));
                }
            }
            return result;
        }

        runtime::types::recursion_guard _guard;
    };

    values::type value::infer_type() const
    {
        type_inference_visitor visitor;
        return boost::apply_visitor(visitor, *this);
    }

    array value::to_array(bool convert_hash)
    {
        // If already an array, return it
        if (as<values::array>()) {
            return move_as<values::array>();
        }

        array result;

        // Check for hash
        auto hash_ptr = as<values::hash>();
        if (convert_hash && hash_ptr) {
            // Turn the hash into an array of [K,V]
            for (auto& kvp : *hash_ptr) {
                array element;
                element.emplace_back(kvp.key());
                element.emplace_back(kvp.value());
                result.emplace_back(rvalue_cast(element));
            }
        } else if (auto iterator = as<values::iterator>()) {
            // Copy the iteration into the result
            iterator->each([&](auto const* key, auto const& value) {
                if (key) {
                    array kvp;
                    kvp[0] = *key;
                    kvp[1] = value;
                    result.emplace_back(rvalue_cast(kvp));
                } else {
                    result.emplace_back(value);
                }
                return true;
            });
        }
        else if (!is_undef()) {
            // Otherwise, add the value as the only element
            result.emplace_back(rvalue_cast(*this));
        }
        return result;
    }

    struct value_printer : boost::static_visitor<ostream&>
    {
        explicit value_printer(ostream& os) :
            _os(os)
        {
        }

        result_type operator()(bool value) const
        {
            _os << (value ? "true" : "false");
            return _os;
        }

        result_type operator()(values::type const& type) const
        {
            // Don't expand when printing out a value that is a type (display aliases as just the name)
            type.write(_os, false);
            return _os;
        }

        template <typename T>
        result_type operator()(T const& value) const
        {
            _os << value;
            return _os;
        }

    private:
        ostream& _os;
    };

    ostream& operator<<(ostream& os, value const& val)
    {
        return boost::apply_visitor(value_printer(os), val);
    }

    bool equality_visitor::operator()(std::string const& left, std::string const& right) const
    {
        return boost::algorithm::iequals(left, right);
    }

    bool operator==(value const& left, value const& right)
    {
        return boost::apply_visitor(equality_visitor(), left, right);
    }

    bool operator!=(value const& left, value const& right)
    {
        return !boost::apply_visitor(equality_visitor(), left, right);
    }

    void value::each_resource(function<void(runtime::types::resource const&)> const& callback, function<void(string const&)> const& error) const
    {
        namespace pt = puppet::runtime::types;

        // Check for string, type, or array
        if (auto str = as<string>()) {
            auto resource = pt::resource::parse(*str);
            if (!resource) {
                if (error) {
                    error((boost::format("expected a resource string but found \"%1%\".") % *str).str());
                }
                return;
            }
            callback(*resource);
            return;
        } else if (auto type = as<values::type>()) {
            // Check for a resource or klass type
            if (auto resource = boost::get<pt::resource>(type)) {
                if (resource->fully_qualified()) {
                    callback(*resource);
                    return;
                }
            } else if (auto klass = boost::get<pt::klass>(type)) {
                if (!klass->title().empty()) {
                    callback(runtime::types::resource("class", klass->title()));
                    return;
                }
            } else if (auto runtime = boost::get<pt::runtime>(type)) {
                // Check for a collector
                if (runtime->object()) {
                    if (auto collector = boost::get<shared_ptr<collectors::collector>>(*runtime->object())) {
                        for (auto resource : collector->resources()) {
                            callback(resource->type());
                        }
                        return;
                    }
                }
            }
        } else if (auto array = as<values::array>()) {
            // For arrays, recurse on each element
            for (auto& element : *array) {
                element->each_resource(callback, error);
            }
            return;
        }

        if (error) {
            error((boost::format("expected %1% or fully qualified %2% for relationship but found %3%.") %
                pt::string::name() %
                pt::resource::name() %
                infer_type()
            ).str());
        }
    }

    struct json_visitor : boost::static_visitor<json_value>
    {
        json_visitor(json_allocator& allocator) :
            _allocator(allocator)
        {
        }

        result_type operator()(undef const&) const
        {
            json_value value;
            value.SetNull();
            return value;
        }

        result_type operator()(defaulted const&) const
        {
            json_value value;
            value.SetString("default");
            return value;
        }

        result_type operator()(int64_t i) const
        {
            json_value value;
            value.SetInt64(i);
            return value;
        }

        result_type operator()(double d) const
        {
            json_value value;
            value.SetDouble(static_cast<double>(d));
            return value;
        }

        result_type operator()(bool b) const
        {
            json_value value;
            value.SetBool(b);
            return value;
        }

        result_type operator()(string const& s) const
        {
            json_value value;
            value.SetString(StringRef(s.c_str(), s.size()));
            return value;
        }

        result_type operator()(values::regex const& regex) const
        {
            auto const& pattern = regex.pattern();
            json_value value;
            value.SetString(StringRef(pattern.c_str(), pattern.size()));
            return value;
        }

        result_type operator()(values::type const& type) const
        {
            json_value value;
            value.SetString(boost::lexical_cast<string>(type).c_str(), _allocator);
            return value;
        }

        result_type operator()(values::variable const& variable) const
        {
            return boost::apply_visitor(*this, variable.value());
        }

        result_type operator()(values::array const& array) const
        {
            json_value value;
            value.SetArray();
            value.Reserve(array.size(), _allocator);

            for (auto const& element : array) {
                value.PushBack(boost::apply_visitor(*this, *element), _allocator);
            }
            return value;
        }

        result_type operator()(values::hash const& hash) const
        {
            json_value value;
            value.SetObject();

            for (auto const& kvp : hash) {
                value.AddMember(
                    json_value(boost::lexical_cast<string>(kvp.key()).c_str(), _allocator),
                    boost::apply_visitor(*this, kvp.value()),
                    _allocator
                );
            }
            return value;
        }

        result_type operator()(values::iterator const& iterator) const
        {
            json_value result;

            bool is_hash = iterator.value().as<values::hash>();

            if (is_hash) {
                result.SetObject();
            } else {
                result.SetArray();
            }

            // Copy the iteration into the result
            iterator.each([&](auto const* key, auto const& value) {
                if (key) {
                    result.AddMember(
                        json_value(boost::lexical_cast<string>(*key).c_str(), _allocator),
                        boost::apply_visitor(*this, value),
                        _allocator
                    );
                } else {
                    result.PushBack(boost::apply_visitor(*this, value), _allocator);
                }
                return true;
            });
            return result;
        }

     private:
        json_allocator& _allocator;
    };

    json_value value::to_json(json_allocator& allocator) const
    {
        return boost::apply_visitor(json_visitor(allocator), *this);
    }

    void each_code_point(string const& str, function<bool(string)> const& callback, bool reverse)
    {
        // Go through each Unicode code point in the string

        if (reverse) {
            if (str.empty()) {
                return;
            }
            // Because utf8::prior walks backward, we can't use rbegin/rend.
            auto current = &str.back() + 1;
            while (current >= &str.front()) {
                auto end = current;
                utf8::prior(current, &str.front());
                if (!callback(string{ current, end })) {
                    break;
                }
            }
            return;
        }

        auto current = str.begin();
        while (current != str.end()) {
            auto begin = current;
            utf8::next(current, str.end());
            if (!callback(string{ begin, current })) {
                break;
            }
        }
    }

}}}  // namespace puppet::runtime::values
