#include <puppet/compiler/resource.hpp>
#include <puppet/compiler/catalog.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <rapidjson/document.h>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler {

    bool indirect_less::operator()(string const* left, string const* right) const
    {
        return *left < *right;
    }

    types::resource const& resource::type() const
    {
        return _type;
    }

    resource const* resource::container() const
    {
        return _container;
    }

    ast::context const& resource::context() const
    {
        static ast::context empty;
        return _context ? *_context : empty;
    }

    string const& resource::path() const
    {
        static string main = "<main>";
        if (!_context || !_context->tree) {
            return main;
        }
        return _context->tree->path();
    }

    size_t resource::line() const
    {
        if (!_context || !_context->tree) {
            return 0;
        }
        return _context->begin.line();
    }

    bool resource::virtualized() const
    {
        return _vertex_id == numeric_limits<size_t>::max();
    }

    bool resource::exported() const
    {
        return _exported;
    }

    shared_ptr<attribute> resource::get(string const& name) const
    {
        auto it = _attributes.find(name);
        if (it == _attributes.end()) {
            return nullptr;
        }
        return it->second;
    }

    void resource::set(shared_ptr<compiler::attribute> attribute)
    {
        if (!attribute) {
            return;
        }

        _attributes[attribute->name()] = rvalue_cast(attribute);
    }

    bool resource::append(shared_ptr<compiler::attribute> attribute)
    {
        if (!attribute) {
            return true;
        }

        auto it = _attributes.find(attribute->name());
        if (it == _attributes.end()) {
            // Not present, just set
            set(rvalue_cast(attribute));
            return true;
        }

        // Ensure the existing value is an array
        if (!it->second->value().as<values::array>()) {
            return false;
        }

        // If the attribute owns the value (is unique), modify it; otherwise copy it as it is shared
        values::array existing;
        if (it->second->unique()) {
            existing = it->second->value().move_as<values::array>();
        } else {
            existing = *it->second->value().as<values::array>();
        }

        // Append the value to the array
        auto value = attribute->value().to_array();
        existing.insert(existing.end(), std::make_move_iterator(value.begin()), std::make_move_iterator(value.end()));

        // Update the attribute's value and set it
        attribute->value() = rvalue_cast(existing);
        set(rvalue_cast(attribute));
        return true;
    }

    void resource::apply(compiler::attributes const& attributes, bool override)
    {
        // Go through each given attribute
        for (auto& pair : attributes) {
            auto oper = pair.first;
            auto& attribute = pair.second;
            // Check for assignment or append
            if (oper == ast::attribute_operator::assignment) {
                if (!override) {
                    if (auto previous = get(attribute->name())) {
                        auto const& context = previous->name_context();
                        if (attribute->value().is_undef()) {
                            throw evaluation_exception(
                                (boost::format("cannot remove attribute '%1%' from resource %2% that was previously set at %3%:%4%.") %
                                 attribute->name() %
                                 _type %
                                 context.tree->path() %
                                 context.begin.line()
                                ).str(),
                                attribute->name_context());
                        }
                        throw evaluation_exception(
                            (boost::format("cannot override attribute '%1%' because it has already been set for resource %2% at %3%:%4%.") %
                             attribute->name() %
                             _type %
                             context.tree->path() %
                             context.begin.line()
                            ).str(),
                            attribute->name_context());
                    }
                }
                // Set the attribute on the resource
                set(attribute);
            } else if (oper == ast::attribute_operator::append) {
                if (!override) {
                    if (auto previous = get(attribute->name())) {
                        auto const& context = previous->name_context();
                        throw evaluation_exception(
                            (boost::format("cannot append to attribute '%1%' because it has already been set for resource %2% at %3%:%4%.") %
                             attribute->name() %
                             this->type() %
                             context.tree->path() %
                             context.begin.line()
                            ).str(),
                            attribute->name_context());
                    }
                }
                append(attribute);
            } else {
                throw runtime_error("unexpected attribute operator");
            }
        }
    }

    void resource::each_attribute(function<bool(attribute const&)> const& callback) const
    {
        if (!callback) {
            return;
        }

        for (auto const& kvp : _attributes) {
            if (!callback(*kvp.second)) {
                break;
            }
        }
    }

    void resource::tag(string tag)
    {
        boost::to_lower(tag);
        _tags.emplace_back(rvalue_cast(tag));
    }

    tag_set resource::calculate_tags() const
    {
        tag_set tags;
        populate_tags(tags);
        return tags;
    }

    bool resource::is_metaparameter(string const& name)
    {
        static const unordered_set<string> metaparameters = {
            "alias",
            "audit",
            "before",
            "loglevel",
            "noop",
            "notify",
            "require",
            "schedule",
            "stage",
            "subscribe",
            "tag"
        };
        return metaparameters.count(name) > 0;
    }

    resource::resource(types::resource type, resource const* container, boost::optional<ast::context> context, bool exported) :
        _type(rvalue_cast(type)),
        _container(container),
        _context(rvalue_cast(context)),
        _vertex_id(numeric_limits<size_t>::max()),
        _exported(exported)
    {
        if (_container && _type.is_stage()) {
            throw runtime_error("stages cannot have a container.");
        }
        if (_context && _context->tree) {
            _tree = _context->tree->shared_from_this();
        }
    }

    json_value resource::to_json(json_allocator& allocator, compiler::catalog const& catalog) const
    {
        json_value value;
        value.SetObject();

        auto& type_name = _type.type_name();
        auto& title = _type.title();

        // Write out the type and title
        value.AddMember("type", rapidjson::StringRef(type_name.c_str(), type_name.size()), allocator);
        value.AddMember("title", rapidjson::StringRef(title.c_str(), title.size()), allocator);

        // Write out the tags
        auto tags = calculate_tags();
        json_value tags_array;
        tags_array.SetArray();
        for (auto& tag : tags) {
            tags_array.PushBack(json_value(rapidjson::StringRef(tag->c_str()), tag->size()), allocator);
        }
        value.AddMember("tags", rvalue_cast(tags_array), allocator);

        // Write out the file and line
        if (_context) {
            auto const& path = this->path();
            value.AddMember("file", rapidjson::StringRef(path.c_str(), path.size()), allocator);
            value.AddMember("line", static_cast<uint64_t>(line()), allocator);
        }

        // Write out whether or not the resource is exported
        value.AddMember("exported", _exported, allocator);

        // Write out the parameters
        json_value parameters;
        parameters.SetObject();
        for (auto& attribute : _attributes) {
            auto const& name = attribute.first;
            auto const& value = attribute.second->value();

            // Do not write any values set to undef
            if (value.is_undef()) {
                continue;
            }

            // Do not write out relationship metaparameters (sourced from dependency graph below)
            if (name == "before" || name == "notify" || name == "require"  || name == "subscribe") {
                continue;
            }

            parameters.AddMember(
                rapidjson::StringRef(name.c_str(), name.size()),
                value.to_json(allocator),
                allocator);
        }

        // Add the relationship parameters
        add_relationship_parameters(parameters, allocator, catalog);

        if (parameters.MemberCount() > 0) {
            value.AddMember("parameters", rvalue_cast(parameters), allocator);
        }
        return value;
    }

    void resource::add_relationship_parameters(json_value& parameters, json_allocator& allocator, compiler::catalog const& catalog) const
    {
        json_value require_parameter;
        json_value subscribe_parameter;

        require_parameter.SetArray();
        subscribe_parameter.SetArray();

        catalog.each_edge(*this, [&](relationship relation, resource const& target) {
            // Ignore containment edges; those are handled by the catalog
            if (relation == relationship::contains) {
                return true;
            }

            // Since the edges represent those resources this resource depends on, treat before as require and notify as subscribe
            json_value* parameter = nullptr;
            if (relation == relationship::before || relation == relationship::require) {
                parameter = &require_parameter;
            } else if (relation == relationship::notify || relation == relationship::subscribe) {
                parameter = &subscribe_parameter;
            }

            if (!parameter) {
                throw runtime_error("unexpected relationship.");
            }

            // Add the target to the parameter
            parameter->PushBack(json_value(boost::lexical_cast<string>(target.type()).c_str(), allocator), allocator);
            return true;
        });

        if (require_parameter.Size() > 0) {
            parameters.AddMember("require", rvalue_cast(require_parameter), allocator);
        }
        if (subscribe_parameter.Size() > 0) {
            parameters.AddMember("subscribe", rvalue_cast(subscribe_parameter), allocator);
        }
    }

    void resource::realize(size_t vertex_id)
    {
        _vertex_id = vertex_id;

        // The resource has been realized, create the tags now
        bool is_class = _type.is_class();

        // Perform auto tagging
        if (is_class) {
            _tags.emplace_back("class");
        }

        auto name = boost::to_lower_copy(is_class ? _type.title() : _type.type_name());
        boost::split_iterator<std::string::iterator> end;
        size_t parts = 0;
        for (auto it = boost::make_split_iterator(name, boost::first_finder("::", boost::is_equal())); it != end; ++it) {
            if (!*it) {
                continue;
            }
            _tags.emplace_back(it->begin(), it->end());
            ++parts;
        }

        // If the name had more than one part, add the entire name too (otherwise it was already added)
        if (parts > 1) {
            _tags.emplace_back(rvalue_cast(name));
        }
    }

    size_t resource::vertex_id() const
    {
        return _vertex_id;
    }

    void resource::populate_tags(tag_set& tags) const
    {
        // First add what is in the tags list
        for (auto const& tag : _tags) {
            tags.insert(&tag);
        }

        // Next add what is in the tag metaparameter
        auto attribute = get("tag");
        if (attribute) {
            if (auto array = attribute->value().as<values::array>()) {
                for (auto const& element : *array) {
                    if (auto tag = element->as<string>()) {
                        tags.insert(tag);
                    }
                }
            }
        }

        // If there's a container, add its tags
        // REVISIT: it would be far more efficient not to have to duplicate these at every resource, but that
        //          requires catalog format improvements
        if (_container) {
            _container->populate_tags(tags);
        }
    }

}}  // namespace puppet::compiler
