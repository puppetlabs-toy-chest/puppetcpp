#include <puppet/runtime/values/value.hpp>
#include <puppet/compiler/evaluation/collectors/collector.hpp>
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

    value& value::operator=(values::wrapper<value>&& wrapper)
    {
        value_base::operator=(rvalue_cast(static_cast<value_base&>(wrapper.get())));
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

    struct truthy_visitor : boost::static_visitor<bool>
    {
        result_type operator()(undef const&) const
        {
            return false;
        }

        result_type operator()(bool val) const
        {
            return val;
        }

        result_type operator()(variable const& var) const
        {
            return var.value().is_truthy();
        }

        template <typename T>
        result_type operator()(T const&) const
        {
            return true;
        }
    };

    bool value::is_truthy() const
    {
        return boost::apply_visitor(truthy_visitor(), *this);
    }

    struct type_visitor : boost::static_visitor<values::type>
    {
        result_type operator()(undef const&) const
        {
            return types::undef();
        }

        result_type operator()(defaulted const&) const
        {
            return types::defaulted();
        }

        result_type operator()(int64_t) const
        {
            return types::integer();
        }

        result_type operator()(long double) const
        {
            return types::floating();
        }

        result_type operator()(bool) const
        {
            return types::boolean();
        }

        result_type operator()(string const&) const
        {
            return types::string();
        }

        result_type operator()(values::regex const&) const
        {
            return types::regexp();
        }

        result_type operator()(type const& t) const
        {
            return types::type(make_unique<type>(t));
        }

        result_type operator()(variable const& var) const
        {
            return boost::apply_visitor(*this, var.value());
        }

        result_type operator()(array const&) const
        {
            return types::array(make_unique<type>(types::any()));
        }

        result_type operator()(hash const&) const
        {
            return types::hash(make_unique<type>(types::any()), make_unique<type>(types::any()));
        }
    };

    values::type value::get_type() const
    {
        return boost::apply_visitor(type_visitor(), *this);
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
                element.emplace_back(*kvp.first);
                element.emplace_back(*kvp.second);
                result.emplace_back(rvalue_cast(element));
            }
        } else if (!is_undef()) {
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
            boost::apply_visitor(*this, type);
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
                get_type()
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

        result_type operator()(long double d) const
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

            for (auto const& element : hash) {
                value.AddMember(
                    json_value(boost::lexical_cast<string>(*element.first).c_str(), _allocator),
                    boost::apply_visitor(*this, *element.second),
                    _allocator);
            }
            return value;
        }

     private:
        json_allocator& _allocator;
    };

    json_value value::to_json(json_allocator& allocator) const
    {
        return boost::apply_visitor(json_visitor(allocator), *this);
    }

    void enumerate_string(string const& str, function<bool(string)> const& callback)
    {
        // Go through each Unicode code point in the string
        auto current = str.begin();
        while (current != str.end()) {
            auto begin = current;
            utf8::next(current, str.end());
            if (!callback(string(begin, current))) {
                break;
            }
        }
    }

}}}  // namespace puppet::runtime::values
