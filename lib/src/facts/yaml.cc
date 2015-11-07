#include <puppet/facts/yaml.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/eventhandler.h>
#include <fstream>

using namespace std;
using namespace puppet::compiler::lexer;
using namespace puppet::runtime;
using namespace YAML;

namespace puppet { namespace facts {

    yaml_parse_exception::yaml_parse_exception(string const& message, string path, size_t line, size_t column, string text) :
        runtime_error(message),
        _path(rvalue_cast(path)),
        _line(line),
        _column(column),
        _text(rvalue_cast(text))
    {
    }

    string const& yaml_parse_exception::path() const
    {
        return _path;
    }

    size_t yaml_parse_exception::line() const
    {
        return _line;
    }

    size_t yaml_parse_exception::column() const
    {
        return _column;
    }

    string const& yaml_parse_exception::text() const
    {
        return _text;
    }

    yaml::yaml(string const& path)
    {
        // Parse the fact file
        ifstream stream(path);
        if (!stream) {
            throw yaml_parse_exception((boost::format("cannot open facts file '%1%'.") % path).str());
        }
        try {
            Node node = YAML::Load(stream);
            for (auto const& kvp : node) {
                store(kvp.first.as<string>(), kvp.second);
            }
        } catch (Exception& ex) {
            string text;
            size_t column;
            tie(text, column) = get_text_and_column(stream, ex.mark.pos);
            throw yaml_parse_exception((boost::format("failed parsing facts: %1%.") % ex.msg).str(), path, ex.mark.line + 1, column, rvalue_cast(text));
        }
    }

    shared_ptr<values::value const> yaml::lookup(string const& name)
    {
        // Check the cache for the value
        auto it = _cache.find(name);
        if (it != _cache.end()) {
            _accessed[name] = it->second;
            return it->second;
        }
        return nullptr;
    }

    void yaml::each(bool accessed, function<bool(string const&, shared_ptr<values::value const> const&)> const& callback)
    {
        // Default to the entire cache
        auto ptr = &_cache;
        if (accessed) {
            ptr = &_accessed;
        }

        // Enumerate all of the items in the collection
        for (auto kvp : *ptr) {
            if (!callback(kvp.first, kvp.second)) {
                break;
            }
        }
    }

    void yaml::store(string const& name, Node const& node, values::value* parent)
    {
        values::value value;

        if (node.IsScalar()) {
            bool bool_val;
            int64_t int_val;
            double double_val;
            if (convert<bool>::decode(node, bool_val)) {
                value = bool_val;
            } else if (convert<int64_t>::decode(node, int_val)) {
                value = int_val;
            } else if (convert<double>::decode(node, double_val)) {
                value = static_cast<long double>(double_val);
            } else {
                // NOTE: as<T> incorrectly returns const T (bug in yaml-cpp), so make the copy explicit
                string copy = node.as<string>();
                value = rvalue_cast(copy);
            }
        } else if (node.IsSequence()) {
            value = values::array();
            for (auto const& child : node) {
                store(string(), child, &value);
            }
        } else if (node.IsMap()) {
            value = values::hash();
            for (auto const& child : node) {
                store(child.first.as<string>(), child.second, &value);
            }
        }

        // If parent, add to array or map
        if (parent) {
            // boost::get is used here because we know the parent is an array or hash and not a variable
            if (auto ptr = boost::get<values::array>(parent)) {
                ptr->emplace_back(rvalue_cast(value));
            } else if (auto ptr = boost::get<values::hash>(parent)) {
                ptr->emplace(make_pair(name, rvalue_cast(value)));
            }
        } else {
            _cache.emplace(make_pair(boost::to_lower_copy(name), std::make_shared<values::value>(rvalue_cast(value))));
        }
    }

}}  // namespace puppet::facts
