#include <puppet/runtime/catalog.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    resource::resource(runtime::catalog& catalog, types::resource type, shared_ptr<string> path, size_t line, bool exported) :
        _catalog(catalog),
        _type(rvalue_cast(type)),
        _path(rvalue_cast(path)),
        _line(line),
        _exported(exported)
    {
        if (!_path) {
            throw runtime_error("expected path.");
        }
    }

    runtime::catalog const& resource::catalog() const
    {
        return _catalog;
    }

    types::resource const& resource::type() const
    {
        return _type;
    }

    string const& resource::path() const
    {
        return *_path;
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
        _tags.emplace(rvalue_cast(tag));
    }

    void resource::set_parameter(string const& name, lexer::position const& name_position, values::value value, lexer::position const& value_position, bool override)
    {
        // Handle metaparameters
        if (handle_metaparameter(name, name_position, value, value_position)) {
            return;
        }

        store_parameter(name, name_position, rvalue_cast(value), override);
    }

    bool resource::remove_parameter(string const& name)
    {
        return _parameters.erase(name) > 0;
    }

    void resource::store_parameter(string const& name, lexer::position const& name_position, values::value value, bool override)
    {
        auto it = _parameters.find(name);
        if (it != _parameters.end()) {
            if (!override) {
                throw evaluation_exception(name_position, (boost::format("attribute '%1%' has already been set for resource %2%.") % name % _type).str());
            }
            it->second = rvalue_cast(value);
            return;
        }
        _parameters.emplace(make_pair(name, rvalue_cast(value)));
    }

    bool resource::handle_metaparameter(string const& name, lexer::position const& name_position, values::value& value, lexer::position const& value_position)
    {
        if (name == "alias") {
            create_alias(value, value_position);
            store_parameter(name, name_position, rvalue_cast(value), false);
            return true;
        } else if (name == "audit") {
            throw evaluation_exception(name_position, "the resource metaparameter 'audit' is not supported.");
        }
        return false;
    }

    void resource::create_alias(values::value const& name, lexer::position const& position)
    {
        if (auto alias = as<string>(name)) {
            if (alias->empty()) {
                throw evaluation_exception(position, "alias name cannot be empty.");
            }
            // Alias this resource
            if (!_catalog.alias_resource(_type, *alias)) {
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

    resource* catalog::find_resource(types::resource const& resource)
    {
        if (resource.type_name().empty() || resource.title().empty()) {
            return nullptr;
        }

        // Check for an alias first
        auto aliases = _aliases.find(resource.type_name());
        if (aliases != _aliases.end()) {
            auto alias = aliases->second.find(resource.title());
            if (alias != aliases->second.end()) {
                return alias->second;
            }
        }

        // Otherwise find the resource type and title
        auto resources = _resources.find(resource.type_name());
        if (resources == _resources.end()) {
            return nullptr;
        }
        auto it = resources->second.find(resource.title());
        if (it == resources->second.end()) {
            return nullptr;
        }
        return &it->second;
    }

    bool catalog::alias_resource(types::resource const& resource, string const& alias)
    {
        // Check if a resource with the alias name already exists
        if (find_resource(types::resource(resource.type_name(), alias))) {
            return false;
        }

        // Find the resource being aliased
        auto existing = find_resource(resource);
        if (!existing) {
            return false;
        }

        _aliases[resource.type_name()].emplace(make_pair(alias, existing));
        return true;
    }

    runtime::resource* catalog::add_resource(types::resource resource, shared_ptr<string> path, size_t line, bool exported)
    {
        if (resource.type_name().empty() || resource.title().empty()) {
            return nullptr;
        }

        string title = resource.title();
        auto result = _resources[resource.type_name()].emplace(make_pair(rvalue_cast(title), runtime::resource(*this, rvalue_cast(resource), rvalue_cast(path), line, exported)));
        if (!result.second) {
            return nullptr;
        }
        return &result.first->second;
    }

}}  // namespace puppet::runtime
