#include <puppet/runtime/catalog.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    resource::resource(runtime::catalog& catalog, string type, string title, string file, size_t line, bool exported) :
        _catalog(catalog),
        _type(std::move(type)),
        _title(std::move(title)),
        _file(std::move(file)),
        _line(line),
        _exported(exported)
    {
    }

    runtime::catalog const& resource::catalog() const
    {
        return _catalog;
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

    void resource::set_parameter(string const& name, token_position const& name_position, values::value value, token_position const& value_position, bool override)
    {
        // Handle metaparameters
        if (handle_metaparameter(name, name_position, value, value_position)) {
            return;
        }

        store_parameter(name, name_position, std::move(value), override);
    }

    bool resource::remove_parameter(string const& name)
    {
        return _parameters.erase(name) > 0;
    }

    types::resource resource::create_reference() const
    {
        return types::resource(_type, _title);
    }

    void resource::store_parameter(string const& name, token_position const& name_position, values::value value, bool override)
    {
        auto it = _parameters.find(name);
        if (it != _parameters.end()) {
            if (!override) {
                throw evaluation_exception(name_position, (boost::format("attribute '%1%' has already been set for resource %2%.") % name % create_reference()).str());
            }
            it->second = std::move(value);
            return;
        }
        _parameters.emplace(make_pair(name, std::move(value)));
    }

    bool resource::handle_metaparameter(string const& name, token_position const& name_position, values::value& value, token_position const& value_position)
    {
        if (name == "alias") {
            create_alias(value, value_position);
            store_parameter(name, name_position, std::move(value), false);
            return true;
        } else if (name == "audit") {
            throw evaluation_exception(name_position, "the resource metaparameter 'audit' is not supported.");
        }
        return false;
    }

    void resource::create_alias(values::value const& name, token_position const& position)
    {
        if (auto alias = as<string>(name)) {
            if (alias->empty()) {
                throw evaluation_exception(position, "alias name cannot be empty.");
            }
            // Alias this resource
            if (!_catalog.alias_resource(_type, _title, *alias)) {
                throw evaluation_exception(position, (boost::format("a %1% resource with name or alias '%2%' already exists in the catalog.") % _type % *alias).str());
            }
            return;
        }
        if (auto aliases = as<values::array>(name)) {
            // For arrays, recurse on each element
            for (auto& element : *aliases) {
                create_alias(element, position);
            }
            return;
        }
        throw evaluation_exception(position, (boost::format("expected %1% for alias name but found %2%.") % types::string::name() % get_type(name)).str());
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
        // Check for an alias first
        auto aliases = _aliases.find(type);
        if (aliases != _aliases.end()) {
            auto alias = aliases->second.find(title);
            if (alias != aliases->second.end()) {
                return alias->second;
            }
        }

        // Otherwise find the resource type and title
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

    bool catalog::alias_resource(string const& type, string const& title, string const& alias)
    {
        // Check if a resource with the alias name already exists
        if (find_resource(type, alias)) {
            return false;
        }

        // Find the resource being aliased
        auto resource = find_resource(type, title);
        if (!resource) {
            return false;
        }

        _aliases[type].emplace(make_pair(alias, resource));
        return true;
    }

    resource* catalog::add_resource(string const& type, string const& title, string const& file, size_t line, bool exported)
    {
        // Add a new resource
        auto result = _resources[type].emplace(make_pair(title, resource(*this, type, title, file, line, exported)));
        if (!result.second) {
            return nullptr;
        }
        return &result.first->second;
    }

}}  // namespace puppet::runtime
