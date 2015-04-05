#include <puppet/runtime/value.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace puppet { namespace runtime {

    ostream& operator<<(ostream& os, undef const&)
    {
        return os;
    }

    regex::regex()
    {
    }

    regex::regex(string pattern) :
        _regex(pattern)
    {
        _pattern = std::move(pattern);
    }

    string const& regex::pattern() const
    {
        return _pattern;
    }

    string& regex::pattern()
    {
        return _pattern;
    }

    std::regex const& regex::value() const
    {
        return _regex;
    }

    std::regex& regex::value()
    {
        return _regex;
    }

    ostream& operator<<(ostream& os, regex const& regx)
    {
        os << '/' << regx.pattern() << '/';
        return os;
    }

    type_kind get_type_kind(string const& name)
    {
        static unordered_map<string, type_kind> kinds = {
            { "Any",            type_kind::any },
            { "Scalar",         type_kind::scalar },
            { "Numeric",        type_kind::numeric },
            { "Integer",        type_kind::integer },
            { "Float",          type_kind::floating },
            { "String",         type_kind::string },
            { "Enum",           type_kind::enumeration },
            { "Pattern",        type_kind::pattern },
            { "Boolean",        type_kind::boolean },
            { "Regexp",         type_kind::regexp },
            { "Collection",     type_kind::collection },
            { "Array",          type_kind::array },
            { "Hash",           type_kind::hash },
            { "Variant",        type_kind::variant },
            { "Optional",       type_kind::optional },
            { "CatalogEntry",   type_kind::catalog_entry },
            { "Resource",       type_kind::resource },
            { "Class",          type_kind::klass },
            { "Undef",          type_kind::undef },
            { "Data",           type_kind::data },
            { "Callable",       type_kind::callable },
            { "Type",           type_kind::type },
            { "Runtime",        type_kind::runtime },
            { "Default",        type_kind::default_ }
        };

        auto it = kinds.find(name);
        if (it != kinds.end()) {
            return it->second;
        }
        return type_kind::unknown;
    }

    ostream& operator<<(ostream& os, type_kind kind)
    {
        static unordered_map<type_kind, string, boost::hash<type_kind>> kinds = {
            { type_kind::any,               "Any" },
            { type_kind::scalar,            "Scalar"  },
            { type_kind::numeric,           "Numeric"  },
            { type_kind::integer,           "Integer"  },
            { type_kind::floating,          "Float" },
            { type_kind::string,            "String" },
            { type_kind::enumeration,       "Enum" },
            { type_kind::pattern,           "Pattern" },
            { type_kind::boolean,           "Boolean" },
            { type_kind::regexp,            "Regexp" },
            { type_kind::collection,        "Collection" },
            { type_kind::array,             "Array" },
            { type_kind::hash,              "Hash" },
            { type_kind::variant,           "Variant" },
            { type_kind::optional,          "Optional" },
            { type_kind::catalog_entry,     "CatalogEntry" },
            { type_kind::resource,          "Resource" },
            { type_kind::klass,             "Class" },
            { type_kind::undef,             "Undef" },
            { type_kind::data,              "Data" },
            { type_kind::callable,          "Callable" },
            { type_kind::type,              "Type" },
            { type_kind::runtime,           "Runtime" },
            { type_kind::default_,          "Default" }
        };

        auto it = kinds.find(kind);
        if (it != kinds.end()) {
            os << it->second;
        }
        return os;
    }

    namespace hack {
        bool unsafe_equals(void const* left, void const* right)
        {
            return equals(*static_cast<value const*>(left), *static_cast<value const*>(right));
        }
    }  // namespace hack

    struct value_visitor : boost::static_visitor<ostream&>
    {
        explicit value_visitor(ostream& os) :
            _os(os)
        {
        }

        result_type operator()(bool val) const
        {
            _os << (val ? "true" : "false");
            return _os;
        }

        template <typename T>
        result_type operator()(T const& t) const
        {
            _os << t;
            return _os;
        }

    private:
        ostream& _os;
    };

    ostream& operator<<(ostream& os, value const& val)
    {
        return boost::apply_visitor(value_visitor(os), val);
    }

    ostream& operator<<(ostream& os, type const& t)
    {
        os << t.kind();
        if (!t.parameters().empty()) {
            os << '[';
            bool first = true;
            for (auto const& parameter : t.parameters()) {
                if (first) {
                    first = false;
                } else {
                    os << ", ";
                }
                os << parameter;
            }
            os << ']';
        }
        return os;
    }

    ostream& operator<<(ostream& os, variable const& var)
    {
        os << var.value();
        return os;
    }

    ostream& operator<<(ostream& os, array const& arr)
    {
        os << '[';
        bool first = true;
        for (auto const& element : arr) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << element;
        }
        os << ']';
        return os;
    }

    ostream& operator<<(ostream& os, hash const& h)
    {
        os << '{';
        bool first = true;
        for (auto const& element : h) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << element.first << " => " << element.second;
        }
        os << '}';
        return os;
    }

    value const& dereference(value const& val)
    {
        auto ptr = boost::get<variable>(&val);
        if (ptr) {
            return ptr->value();
        }
        return val;
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
            return is_truthy(var.value());
        }

        template <typename T>
        result_type operator()(T const&) const
        {
            return true;
        }
    };

    bool is_truthy(value const& val)
    {
        return boost::apply_visitor(truthy_visitor(), val);
    }

    struct type_visitor : boost::static_visitor<type_kind>
    {
        result_type operator()(undef const&) const
        {
            return type_kind::undef;
        }

        result_type operator()(int64_t) const
        {
            return type_kind::integer;
        }

        result_type operator()(long double) const
        {
            return type_kind::floating;
        }

        result_type operator()(bool) const
        {
            return type_kind::boolean;
        }

        result_type operator()(string const&) const
        {
            return type_kind::string;
        }

        result_type operator()(runtime::regex const&) const
        {
            return type_kind::regexp;
        }

        result_type operator()(type const&) const
        {
            return type_kind::type;
        }

        result_type operator()(variable const& var) const
        {
            return boost::apply_visitor(*this, var.value());
        }

        result_type operator()(runtime::array const&) const
        {
            return type_kind::array;
        }

        result_type operator()(hash const&) const
        {
            return type_kind::hash;
        }
    };

    type get_type(value const& val)
    {
        auto kind = boost::apply_visitor(type_visitor(), val);
        return type(kind);
    }

    array to_array(value const& val)
    {
        // If already an array, return a copy
        auto array_ptr = boost::get<array>(&val);
        if (array_ptr) {
            return *array_ptr;
        }

        array result;

        // Check for hash
        auto hash_ptr = boost::get<hash>(&val);
        if (hash_ptr) {
            // Turn the hash into an array of [K,V]
            for (auto& kvp : *hash_ptr) {
                array element;
                element.emplace_back(kvp.first);
                element.emplace_back(kvp.second);
                result.emplace_back(std::move(element));
            }
        } else {
            // Otherwise, add the value as the only element
            result.emplace_back(val);
        }
        return result;
    }

    bool operator==(undef const&, undef const&)
    {
        return true;
    }

    bool operator==(regex const& left, regex const& right)
    {
        return left.pattern() == right.pattern();
    }

    bool operator==(type const& left, type const& right)
    {
        // TODO: validate parameters
        return left.kind() == right.kind();
    }

    bool operator==(variable const& left, variable const& right)
    {
        // Optimization: if both variables point to the same value, they are equal
        if (&left.value() == &right.value()) {
            return true;
        }
        return equals(left.value(), right.value());
    }

    bool operator==(array const& left, array const& right)
    {
        if (left.size() != right.size()) {
            return false;
        }
        for (size_t i = 0; i < left.size(); ++i) {
            if (!equals(left[i], right[i])) {
                return false;
            }
        }
        return true;
    }

    bool operator==(hash const& left, hash const& right)
    {
        if (left.size() != right.size()) {
            return false;
        }
        for (auto const& element : left) {
            // Other hash must have the same key
            auto other = right.find(element.first);
            if (other == right.end()) {
                return false;
            }
            // Values must be equal
            if (!equals(element.second, other->second)) {
                return false;
            }
        }
        return true;
    }

    equality_visitor::result_type equality_visitor::operator()(string const& left, string const& right) const
    {
        return boost::algorithm::iequals(left, right);
    }

    bool equals(value const& left, value const& right)
    {
        return boost::apply_visitor(equality_visitor(), left, right);
    }

}}  // namespace puppet::runtime
