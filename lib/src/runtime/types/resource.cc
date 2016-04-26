#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>
#include <unordered_set>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    resource const resource::instance;

    resource::resource(std::string type_name, std::string title) :
        _type_name(rvalue_cast(type_name)),
        _title(rvalue_cast(title))
    {
        // Make the type name lowercase
        boost::to_lower(_type_name);

        // Now uppercase every start of a type name
        boost::split_iterator<std::string::iterator> end;
        for (auto it = boost::make_split_iterator(_type_name, boost::first_finder("::", boost::is_equal())); it != end; ++it) {
            if (!*it) {
                continue;
            }
            auto range = boost::make_iterator_range(it->begin(), it->begin() + 1);
            boost::to_upper(range);
        }
    }

    std::string const& resource::type_name() const
    {
        return _type_name;
    }

    std::string const& resource::title() const
    {
        return _title;
    }

    bool resource::fully_qualified() const
    {
        return !_type_name.empty() && !_title.empty();
    }

    bool resource::is_class() const
    {
        return _type_name == "Class";
    }

    bool resource::is_stage() const
    {
        return _type_name == "Stage";
    }

    bool resource::is_builtin(std::string const& name)
    {
        // TODO: remove this member once built-in types can be defined
        static const unordered_set<std::string> builtin_types = {
            "Augeas",
            "Class",
            "Computer",
            "Cron",
            "Exec",
            "File",
            "Filebucket",
            "Group",
            "Host",
            "Interface",
            "5klogin",
            "Macauthorization",
            "Mailalias",
            "Maillist",
            "Mcx",
            "Mount",
            "Nagios_command",
            "Nagios_contact",
            "Nagios_contactgroup",
            "Nagios_host",
            "Nagios_hostdependency",
            "Nagios_hostescalation",
            "Nagios_hostextinfo",
            "Nagios_hostgroup",
            "Nagios_service",
            "Nagios_servicedependency",
            "Nagios_serviceescalation",
            "Nagios_serviceextinfo",
            "Nagios_servicegroup",
            "Nagios_timeperiod",
            "Node",
            "Notify",
            "Package",
            "Resources",
            "Router",
            "Schedule",
            "Scheduled_task",
            "Selboolean",
            "Selmodule",
            "Service",
            "Ssh_authorized_key",
            "Sshkey",
            "Stage",
            "Tidy",
            "User",
            "Vlan",
            "Yumrepo",
            "Zfs",
            "Zone",
            "Zpool"
        };
        return builtin_types.count(name) > 0;
    }

    char const* resource::name()
    {
        return "Resource";
    }

    values::type resource::generalize() const
    {
        return *this;
    }

    bool resource::is_instance(values::value const& value, recursion_guard& guard) const
    {
        // Check for type
        auto ptr = value.as<values::type>();
        if (!ptr) {
            return false;
        }
        // Check for resource type
        auto resource_ptr = boost::get<resource>(ptr);
        if (!resource_ptr) {
            return false;
        }
        // If no type, the given value is a 'resource'
        if (_type_name.empty()) {
            return true;
        }
        // Check type name
        if (_type_name != resource_ptr->type_name()) {
            return false;
        }
        return _title.empty() || _title == resource_ptr->title();
    }

    bool resource::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        auto resource = boost::get<types::resource>(&other);
        if (!resource) {
            return false;
        }
        return _type_name.empty() || (_type_name == resource->type_name() && (_title.empty() || _title == resource->title()));
    }

    void resource::write(ostream& stream, bool expand) const
    {
        if (_type_name.empty()) {
            stream << resource::name();
            return;
        }
        stream << _type_name;
        if (_title.empty()) {
            return;
        }
        stream << "[" << _title << "]";
    }

    boost::optional<resource> resource::parse(std::string const& specification)
    {
        static utility::regex specification_regex{ R"(^((?:(?:::)?[A-Z]\w*)+)\[([^\]]+)\]$)" };

        utility::regex::regions regions;
        if (!specification_regex.match(specification, &regions) || regions.count() != 3) {
            return boost::none;
        }

        auto title = regions.substring(specification, 2);
        boost::trim(title);
        // Strip quotes if present in the title
        if (!title.empty()) {
            if ((title.front() == '"' && title.back() == '"') ||
                (title.front() == '\'' && title.back() == '\'')) {
                title = title.substr(1, title.size() - 2);
            }
        }
        return resource(regions.substring(specification, 1), rvalue_cast(title));
    }

    ostream& operator<<(ostream& os, resource const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(resource const& left, resource const& right)
    {
        return left.type_name() == right.type_name() && left.title() == right.title();
    }

    bool operator!=(resource const& left, resource const& right)
    {
        return !(left == right);
    }

    size_t hash_value(resource const& type)
    {
        static const size_t name_hash = boost::hash_value(resource::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.type_name());
        boost::hash_combine(seed, type.title());
        return seed;
    }

}}}  // namespace puppet::runtime::types
