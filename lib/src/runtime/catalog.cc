#include <puppet/runtime/catalog.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    resource::resource(string type, string title, string file, size_t line, bool exported) :
        _type(std::move(type)),
        _title(std::move(title)),
        _file(std::move(file)),
        _line(line),
        _exported(exported)
    {
    }

    string const& resource::type() const
    {
        return _type;
    }

    string const& resource::title() const
    {
        return _title;
    }

    string const& resource::file() const
    {
        return _file;
    }

    size_t resource::line() const
    {
        return _line;
    }

    unordered_set<string> const& resource::tags() const
    {
        return _tags;
    }

    unordered_map<string, value> const& resource::parameters() const
    {
        return _parameters;
    }

    bool resource::exported() const
    {
        return _exported;
    }

    void resource::add_tag(string tag)
    {
        _tags.emplace(std::move(tag));
    }

    bool resource::set_parameter(std::string name, value val)
    {
        return _parameters.emplace(make_pair(std::move(name), std::move(val))).second;
    }

    catalog::catalog()
    {
    }

    catalog::resource_map const& catalog::resources() const
    {
        return _resources;
    }

    resource const* catalog::find_resource(string const& type, string const& title) const
    {
        auto resources = _resources.find(type);
        if (resources == _resources.end()) {
            return nullptr;
        }
        auto it = resources->second.find(title);
        if (it == resources->second.end()) {
            return nullptr;
        }
        return &it->second;
    }

    resource* catalog::find_resource(string const& type, string const& title)
    {
        auto resources = _resources.find(type);
        if (resources == _resources.end()) {
            return nullptr;
        }
        auto it = resources->second.find(title);
        if (it == resources->second.end()) {
            return nullptr;
        }
        return &it->second;
    }

    resource* catalog::add_resource(string const& type, string const& title, string const& file, size_t line, bool exported)
    {
        auto result = _resources[type].emplace(make_pair(title, resource(type, title, file, line, exported)));
        if (!result.second) {
            return nullptr;
        }
        return &result.first->second;
    }

}}  // namespace puppet::runtime
